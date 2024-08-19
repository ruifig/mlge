#pragma once

#include "Common.h"
#include "Logging.h"

namespace cz
{

template<typename T>
inline void sharedPtrDeleter(T* obj)
{
	obj->~T();
}

namespace details
{

	class BaseSharedPtrControlBlock
	{
	  public:

		template<typename T>
		T* toObject()
		{
			return reinterpret_cast<T*>(this+1);
		}

		void incStrong()
		{
			++strong;
		}

		void incWeak()
		{
			++weak;
		}

		void decWeak()
		{
			assert(weak > 0);
			--weak;
			if (weak == 0 && strong == 0)
			{
				free(this);
			}
		}

		unsigned int strongRefs() const
		{
			return strong;
		}

		unsigned int weakRefs() const
		{
			return weak;
		}

		/**
		 * Allocates memory for a control block + a T object
		 *
		 * Returns the pointer that can be used to construct a T object with placement new
		 */
		template<typename T>
		static void* allocBlock()
		{
			size_t allocSize = sizeof(BaseSharedPtrControlBlock) + sizeof(T);
			void* basePtr = malloc(allocSize);
			BaseSharedPtrControlBlock* control = new (basePtr) BaseSharedPtrControlBlock;
			return control + 1;
		}

	  protected:
		unsigned int weak = 0;
		unsigned int strong = 0;
	};

	struct SharedPtrDefaultDeleter
	{
		template<typename T>
		void operator()(T* obj) const
		{
			obj->~T();
		}
	};

	template<typename T, typename Deleter = SharedPtrDefaultDeleter>
	class SharedPtrControlBlock : public BaseSharedPtrControlBlock
	{
	  public:

		SharedPtrControlBlock()
		{
			// The only difference between the base object and a specialized one is that that specialized one adds some more methods.
			static_assert(sizeof(*this) == sizeof(BaseSharedPtrControlBlock));
		}

		T* obj()
		{
			if (strong)
			{
				return toObject<T>();
			}
			else
			{
				return nullptr;
			}
		}

		void decStrong()
		{
			assert(strong > 0);
			if (strong == 1)
			{
				Deleter{}(obj());
			}

			--strong;
			if (strong == 0)
			{
				if (weak == 0)
				{
					free(this);
				}
			}
		}

	};
}

/**
 * A rather simplistic std::shared_ptr equivalent, that is faster than std::shared_ptr since it is NOT thread safe.
 * Things to be aware of:
 * 
 * - NOT thread safe. That's the point of this class.
 * - Might not provide all the functions and/or operators std::shared_ptr provides
 * - IMPORTANT: Assumes memory for the object was allocated with BaseSharedPtrControlBlock::allocBlock. this means that when using
 *   the SharedPtr<T>::SharedPtr(U* ptr) constructor, care must be taken that "ptr" was allocated properly. You can do this by:
 *		- Using MakeShared<T>. E.g: SharedPtr<Foo> foo = MakeShared<Foo>();
 *		- Using BaseSharedControl::allocBlock and placement new. e.g:
 *			SharedPtr<Foo> foo(new(details::BaseObjectControlBlock::allocBlock<Foo>()) Foo);
 *	 The reason this constructor is provided is so that SharedPtr can be used with classes whose constructors are private/protected.
 * 
 */
template<typename T, typename Deleter = details::SharedPtrDefaultDeleter>
class SharedPtr
{
  public:

	template<typename U, typename UDeleter>
	friend class WeakPtr;

	template<typename U, typename UDeleter>
	friend class SharedPtr;

	using ControlBlock = details::SharedPtrControlBlock<T, Deleter>;	

	SharedPtr() noexcept {}


	SharedPtr(std::nullptr_t) noexcept
	{
	}

	/**
	 * Constructs the SharedPtr from a previously allocated object.
	 * IMPORTANT: The object's memory MUST have been allocated with BaseSharedPtrControlBlock::allocBlock. See the SharedPtr
	 * class documentation for details.
	 */
	template<typename U>
	explicit SharedPtr(U* ptr) noexcept
	{
		if (ptr)
		{
			static_assert(std::is_convertible_v<U*, T*>);
			void* rawPtr = (reinterpret_cast<uint8_t*>(static_cast<T*>(ptr)) - sizeof(details::BaseSharedPtrControlBlock));
			acquireBlock(reinterpret_cast<ControlBlock*>(rawPtr));
		}
	}

	~SharedPtr()
	{
		releaseBlock();
	}

	SharedPtr(const SharedPtr& other) noexcept
	{
		acquireBlock(other.m_control);
	}

	template<typename U>
	SharedPtr(const SharedPtr<U, Deleter>& other) noexcept
	{
		static_assert(std::is_convertible_v<U*, T*>);
		acquireBlock(other.m_control);
	}

	SharedPtr(SharedPtr&& other) noexcept
	{
		std::swap(m_control, other.m_control);
	}

	template<typename U>
	SharedPtr(SharedPtr<U>&& other) noexcept
	{
		static_assert(std::is_convertible_v<U*, T*>);
		m_control = reinterpret_cast<ControlBlock*>(other.m_control);
		other.m_control = nullptr;
	}

	SharedPtr& operator=(const SharedPtr& other) noexcept
	{
		SharedPtr(other).swap(*this);
		return *this;
	}

	template<typename U>
	SharedPtr& operator=(const SharedPtr<U, Deleter>& other) noexcept
	{
		static_assert(std::is_convertible_v<U*, T*>);
		SharedPtr(other).swap(*this);
		return *this;
	}

	SharedPtr& operator=(SharedPtr&& other) noexcept
	{
		SharedPtr(std::move(other)).swap(*this);
		return *this;
	}

	template<typename U>
	SharedPtr& operator=(SharedPtr<U, Deleter>&& other) noexcept
	{
		static_assert(std::is_convertible_v<U*, T*>);
		SharedPtr(std::move(other)).swap(*this);
		return *this;
	}

	T* operator->() const noexcept
	{
		CZ_CHECK(m_control);
		return m_control->toObject<T>();
	}

	T* get() const noexcept
	{
		if(m_control)
		{
			return m_control->template toObject<T>();
		}
		else
		{
			return nullptr;
		}
	}

	T& operator*() const noexcept
	{
		CZ_CHECK(m_control);
		return *m_control->toObject<T>();
	}

	explicit operator bool() const noexcept
	{
		return m_control ? true : false;
	}

	unsigned int use_count() const noexcept
	{
		return m_control ? m_control->strongRefs() : 0;
	}

	bool unique() const noexcept
	{
		return use_count() == 1;
	}

	void reset() noexcept
	{
		releaseBlock();
		m_control = nullptr;
	}

	void swap(SharedPtr& other) noexcept
	{
		std::swap(m_control, other.m_control);
	}

  private:

	void releaseBlock() noexcept
	{
		if (m_control)
		{
			m_control->decStrong();
		}
	}

	template<typename U>
	void acquireBlock(details::SharedPtrControlBlock<U, Deleter>* control) noexcept
	{
		static_assert(std::is_convertible_v<U*,T*>);
		m_control = reinterpret_cast<ControlBlock*>(control);
		if (m_control)
		{
			m_control->incStrong();
		}
	}

	ControlBlock* m_control = nullptr;
};

template<typename T, typename Deleter = details::SharedPtrDefaultDeleter>
class WeakPtr
{
  public:

	using ControlBlock = details::SharedPtrControlBlock<T>;

	template<typename U, typename UDeleter>
	friend class WeakPtr;

	constexpr WeakPtr() = default;

	WeakPtr(const WeakPtr& other)
	{
		acquireBlock(other.m_control);
	}

	template<typename U>
	WeakPtr(const WeakPtr<U, Deleter>& other) noexcept
	{
		static_assert(std::is_convertible_v<U*, T*>);
		acquireBlock(other.m_control);
	}

	template<typename U>
	WeakPtr(const SharedPtr<U, Deleter>& other)
	{
		static_assert(std::is_convertible_v<U*, T*>);
		acquireBlock(other.m_control);
	}

	WeakPtr(WeakPtr&& other) noexcept
	{
		m_control = other.m_control;
		other.m_control = nullptr;
	}

	template<typename U>
	WeakPtr(WeakPtr<U, Deleter>&& other) noexcept
	{
		static_assert(std::is_convertible_v<U*, T*>);
		m_control = reinterpret_cast<ControlBlock*>(other.m_control);
		other.m_control = nullptr;
	}

	WeakPtr& operator=(const WeakPtr& other)
	{
		WeakPtr(other).swap(*this);
		return *this;
	}

	template<typename U>
	WeakPtr& operator=(const WeakPtr<U, Deleter>& other)
	{
		static_assert(std::is_convertible_v<U*, T*>);
		WeakPtr(other).swap(*this);
		return *this;
	}

	template<typename U>
	WeakPtr& operator=(const SharedPtr<U, Deleter>& other)
	{
		static_assert(std::is_convertible_v<U*, T*>);
		WeakPtr(other).swap(*this);
		return *this;
	}

	template<typename U>
	WeakPtr& operator=(WeakPtr<U, Deleter>&& other)
	{
		static_assert(std::is_convertible_v<U*, T*>);
		WeakPtr(std::move(other)).swap(*this);
		return *this;
	}

	void reset() noexcept
	{
		WeakPtr{}.swap(*this);
	}

	void swap(WeakPtr& other) noexcept
	{
		std::swap(m_control, other.m_control);
	}

	unsigned int use_count() const
	{
		return (m_control) ? m_control->strongRefs() : 0;
	}

	bool expired() const
	{
		return use_count()==0 ? true : false;
	}

	SharedPtr<T, Deleter> lock() const
	{
		if (use_count())
		{
			return SharedPtr<T, Deleter>(m_control->obj());
		}
		else
		{
			return SharedPtr<T, Deleter>();
		}
	}

  private:

	void releaseBlock()
	{
		if (m_control)
		{
			m_control->decWeak();
		}
	}

	template<typename U>
	void acquireBlock(details::SharedPtrControlBlock<U, Deleter>* control)
	{
		static_assert(std::is_convertible_v<U*,T*>);
		m_control = reinterpret_cast<ControlBlock*>(control);
		if (m_control)
		{
			m_control->incWeak();
		}
	}

	ControlBlock* m_control = nullptr;
};


template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&& ... args)
{
	static_assert(!std::is_abstract_v<T>, "Type is abstract.");
	void* ptr = details::BaseSharedPtrControlBlock::allocBlock<T>();
	return SharedPtr<T>(new(ptr) T(std::forward<Args>(args)...));
}

template <class T, class U, typename Deleter>
bool operator==(const SharedPtr<T, Deleter>& left, const SharedPtr<U, Deleter>& right) noexcept
{
	return left.get() == right.get();
}

template <class T, typename Deleter>
bool operator==(const SharedPtr<T, Deleter>& left, std::nullptr_t) noexcept
{
	return left.get() == nullptr;
}

template <class T, typename Deleter>
bool operator==(std::nullptr_t, const SharedPtr<T, Deleter>& right) noexcept
{
	return nullptr == right.get();
}

//The <, <=, >, >=, and != operators are synthesized from operator<=> and operator== respectively.
template<typename T1, typename T2, typename Deleter>
std::strong_ordering operator<=>(const SharedPtr<T1, Deleter>& left, const SharedPtr<T2, Deleter>& right) noexcept
{
	return left.get() <=> right.get();
}


} // namespace cz

