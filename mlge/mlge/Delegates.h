#pragma once

/**
 * Notes for when documenting this:
 * 
 * - Both the target and source can be destroyed at any time. The system takes care of detecting that.
 * 
 */

#include "mlge/Common.h"
#include "crazygaze/core/SharedPtr.h"

namespace mlge
{

namespace details
{
}

class BaseDelegate;

/**
 * Handle returned when registering to a delegate.
 * This takes care of automatically unbinding from the delegate when destroyed.
 */
class DelegateHandle
{
public:

	DelegateHandle() = default;
	DelegateHandle(DelegateHandle&& other)
	{
		std::swap(m_control, other.m_control);
	}

	DelegateHandle& operator=(DelegateHandle&& other)
	{
		if (this != &other)
		{
			unbind();
			std::swap(m_control, other.m_control);
		}

		return *this;
	}

	~DelegateHandle();

	/**
	 * Used internally to solve the problem of unregistering a delegate when the target is destroyed and invalidating delegates
	 * when the source is destroyed.
	 */ 
	struct ControlBlock
	{
		BaseDelegate* source = nullptr;
	};

protected:

	DelegateHandle(const SharedPtr<ControlBlock>& control)
		: m_control(std::move(control))
	{
	}

	void unbind();

	friend class BaseDelegate;

	template<typename... Ts>
	friend class MultiCastDelegate;

	SharedPtr<ControlBlock> m_control;
};

class BaseDelegate
{
protected:
	BaseDelegate() = default;
	virtual ~BaseDelegate() = default;
public:
	virtual void unbind(DelegateHandle::ControlBlock* control) = 0;

};

class BaseMultiCastDelegate : public BaseDelegate
{
public:
	CZ_DELETE_COPY_AND_MOVE(BaseMultiCastDelegate);

protected:


	/**
	 * The only thing we need is a "this" and a function pointer. The templated "call" method is called by the right delegate that
	 * knows the types and therefore casts things appropriately. 
	 *
	 */
	struct Target
	{
		// The "this" pointer
		void* target;
		// The member function to call
		void *memberFunction;

		template<typename TargetType, typename... Ts>
		Target(TargetType* target, void (TargetType::*func)(Ts ...))
		{
			this->target = target;
			// C++ doesn't allow casting from a function pointer to a void*, so first we get a pointer of a pointer, then
			// dereference that.
			// I think that technically this might not valid on all platforms, but the static_asserts _should_ catch that.
			void** ptr = reinterpret_cast<void**>(&func);
			memberFunction = *ptr;
			static_assert(sizeof(void*) == sizeof(void (Target::*)(Ts ...)), "Unexpected size of member function pointers");
		}

		template<typename TargetType, typename... Ts>
		void call(Ts&& ... args)
		{
			auto _this = reinterpret_cast<TargetType*>(target);
			void (TargetType::*_func)(Ts ...);
			void** ptr = reinterpret_cast<void**>(&_func);
			*ptr = memberFunction;
			(_this->*_func)(std::forward<Ts>(args)...);
		}
	};

	BaseMultiCastDelegate() = default;
	~BaseMultiCastDelegate();

	// Using a vector because most likely there won't be that many delegates handles registered to a particular delegate.
	// Also, it means that the order of broadcast is maintained and predictable
	std::vector<std::pair<SharedPtr<DelegateHandle::ControlBlock>, Target>> m_targets;

	/* Set to true when inside broadcast. Allows the code to detect when bind/unbinds from inside broadcast */
	bool m_broadcasting = false;

	virtual void unbind(DelegateHandle::ControlBlock* control) override;
};

template<typename... Ts>
class MultiCastDelegate : public BaseMultiCastDelegate
{
public:

	template<typename TargetType>
	[[nodiscard]] DelegateHandle bind(TargetType* target, void (TargetType::*memberFunction)(Ts... args))
	{
		auto control = MakeShared<DelegateHandle::ControlBlock>();
		control->source = this;
		m_targets.emplace_back(control, Target(target, memberFunction));
		return DelegateHandle(control);
	}

	void broadcast(Ts ... args)
	{
		// Iteration is done with an index and while, to check agains size(), so we can call any new handles added
		// while we are inside broadcast.
		size_t idx = 0;
		m_broadcasting = true;
		while (idx != m_targets.size())
		{
			// If a slot has something, we call it, if not then it means unbind was called from since broadcast, and thus the
			// slot was marked as empty but not removed
			if (m_targets[idx].first)
			{
				m_targets[idx].second.call<Target>(std::forward<Ts>(args)...);
				idx++;
			}
			else
			{
				m_targets.erase(m_targets.begin() + static_cast<int>(idx));
			}
		}
		m_broadcasting = false;
	}

protected:

};


} // namespace mlge
