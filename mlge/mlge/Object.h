#pragma once

#include "mlge/Common.h"
#include "mlge/ObjectPtr.h"
#include "mlge/Misc/LinkedList.h"
#include "crazygaze/core/Logging.h"

namespace mlge
{
//////////////////////////////////////////////////////////////////////////
//				Class
//////////////////////////////////////////////////////////////////////////

// Forward declarations
class MObject;
template<typename T, typename... Args>
ObjectPtr<T> createObject(Args&& ... args);

class Class : public DoublyLinked<Class>
{
  public:

	Class(const char* name, const char* description)
	{
		CZ_CHECK(name[0] == 'M' || name[0] == 'A');

		// Skip the prefix
		m_name = name+1;
		m_description = description;
		ms_all.pushBack(this);
	}

	virtual ~Class()
	{
		ms_all.remove(this);
	}

	const char* getName() const
	{
		return m_name;
	}


	const char* getDescription() const
	{
		return m_description;
	}
	
	/**
	 * Creates an object, and initializes it without parameters
	 */
	virtual ObjectPtr<MObject> createObject() = 0;

	unsigned int getAndIncCounter()
	{
		return m_creationCounter++;
	}

	static Class* find(const char* name);

	template<typename T>
	static Class& get()
	{
		return T::ms_class;
	}

	const Class* getParentClass() const
	{
		return m_parentClass;
	}

	/**
	 * Checks if class is a base class of another.
	 *
	 * @param other The class to check against. If "this" is a base class of "other", it returns true
	 */
	bool isBaseOf(const Class& other) const
	{
		const Class* c = &other;
		while(c)
		{
			if (c == this)
			{
				return true;
			}

			c = c->getParentClass();
		}
		return false;
	}

	bool isAbstract() const
	{
		return m_isAbstract;
	}

	static DoublyLinkedList<Class>::Iterator begin()
	{
		return ms_all.begin();
	}

	static DoublyLinkedList<Class>::Iterator end()
	{
		return ms_all.end();
	}

  protected:

	template<typename T>
	friend ObjectPtr<T> allocObject();

	const char* m_name;
	const char* m_description;
	unsigned int m_creationCounter = 0;
	Class* m_parentClass = nullptr;
	bool m_isAbstract = false;


	inline static DoublyLinkedList<Class> ms_all;
};

class ObjectClass : public Class
{
  public:
	ObjectClass()
		: Class("MObject", "Base object")
	{
		m_parentClass = nullptr;
	}

	virtual ObjectPtr<MObject> createObject() override
	{
		// We can't allocate objects of type "MObject"
		CZ_VERIFY(false);
		return nullptr;
	}
};

//////////////////////////////////////////////////////////////////////////
//				MObject
//////////////////////////////////////////////////////////////////////////

#define MLGE_OBJECT_HELPER_FRIENDS(Type)                    \
	template <typename T, typename... Args>                 \
	friend ObjectPtr<T> mlge::createObject(Args&&... args); \
	friend Class;                                           \
	friend Type##Class;

class Class;

/**
 * Base class for mlge classes that need to be always dynamically allocated.
 *
 * Lifetime of any MObject derived class needs to be controlled with ObjectPtr<T> and WeakObjectPtr<T>.
 *
 * Object creation is done with createObject<T> and the following happens:
 *	- C++ constructor is called
 *	- The "virtual bool preConstruct()" is called.
 *		- If it returns false, object creation is considered as failed.
 *	- If any parameters are passed to createObject<T>, then a "bool construct(...)" that method is called with those parameters.
 *		- If it returns false, object creation is considered as failed.
 *		- createObject<T> will forward any parameters to T::construct, so construct can take any parameters T wants, and thus you
 *		  can think of T::construct as the actual C++ constructor.
 *		- Note that "construct" is typically NOT virtual, because the list of parameters depends on T itself.
 *	- "postConstruct" is called.
 *		- This lets the object type have code called after construction, no matter what "construct" overload was used.
 *
 * Object destruction happens as:
 *	- The virtual "void destruct()" method is called.
 *		- Derived classes can override this and should call "Super::destruct" at end.
 *	- The C++ destructor is called.
 *
 * Derived classes must:
 *	- Implement a preConstruct() and/or construct(...) if necessary.
		- Call Super::preConstruct and/or Super::construct if required.
 *		- Depending on the class hierarchy, this might be virtual or not.
 *		- The reason it needs to be non-virtual is because the list of parameters is up to the class itself, so the signature can
 *		  be different from the one in the base class.
 *	- Override "virtual void destruct()" if required, and if so, call "Super::destruct()" at the end.
 */
class MObject
{
  protected:

	// Provides a way to use make_shared with private/protected constructors
	struct this_is_private
	{
		explicit this_is_private(int) {}
	};

	MObject(const this_is_private&) {}

  public:

	CZ_DELETE_COPY_AND_MOVE(MObject);

	virtual ~MObject();

	Class& getClass()
	{
		CZ_CHECK(m_class);
		return *m_class;
	}

	const Class& getClass() const
	{
		CZ_CHECK(m_class);
		return *m_class;
	}

	const std::string& getObjectName() const
	{
		return m_objectName;
	}

	/**
	 * Called right before the object destructor is called
	 */
	virtual void destruct()
	{
	}

  protected:

	MLGE_OBJECT_HELPER_FRIENDS(Object)

	inline static ObjectClass ms_class;

	/**
	 * Called before `construct`
	 * If this returns false, `construct` is not called and the object creation is considered as failed.
	 *
	 * This is useful for objects that have multiple `construct` overloads and wish some code to always run before the
	 * `construct` call matter what `construct` overload will be called.
	 */
	virtual bool preConstruct()
	{
		return true;
	}

	/**
	 * Called after preConstruct and before postConstruct
	 *
	 * This is intentionally NOT virtual. The object types themselves can have multiple overloads for `construct`, depending on
	 * what parameters they need for construction.
	 */
	bool construct()
	{
		return true;
	}

	/**
	 * If the object is constructed successfully, this called.
	 *
	 * This is useful for objects that have multiple `construct` overloads and wish some code to always run  after the `construct`
	 * call matter what `construct` overload was called.
	 */
	virtual void postConstruct() {}

	/**
	 * Used internally. Putting this in a separate function so that the actual Type::alllocObject function compiles even if
	 * the class is abstract
	 */
	template<typename T>
	static ObjectPtr<T> allocObjectHelper()
	{
		static_assert(!std::is_abstract_v<T>);
		return ObjectPtr<T>(new (details::BaseSharedPtrControlBlock::allocBlock<T>()) T(this_is_private{0}));
	}

	Class* m_class = nullptr;

	/**
	 * Object name
	 */
	std::string m_objectName; // Intentionally using "m_object<VARNAME>" to make sure it doesn't conflict with derived classes
};


#if 0
/**
 * SharedPtr deleter for MObject objects
 */
inline void sharedPtrDeleter(MObject* obj)
{
	obj->destruct();
	obj->~MObject();
}
#endif

/**
 * Put this right before an MLGE object declaration
 *
 * Type - Class name
 * ParentType - Parent class name
 * Description - User friendly description. This is used by the editor
 */
#define MLGE_OBJECT_START(Type, ParentType, Description)    \
	class Type;                                             \
	class Type##Class : public Class                        \
	{                                                       \
	  public:                                               \
		Type##Class()                                       \
			: Class(#Type, Description)                     \
		{                                                   \
			m_parentClass = &Class::get<ParentType>();      \
			init();                                         \
		}                                                   \
		virtual ObjectPtr<MObject> createObject() override; \
                                                            \
	  private:                                              \
		void init();                                        \
	};



#define MLGE_OBJECT_INTERNALS_HELPER_ABSTRACT_YES(Type)

#define MLGE_OBJECT_INTERNALS_HELPER_ABSTRACT_NO(Type)                           \
  private:                                                                       \
	static void _testAbstract()                                                  \
	{                                                                            \
		using AbstractTesting = decltype(new Type(MObject::this_is_private{0})); \
	}                                                                            \
                                                                                 \
  protected:

#define MLGE_OBJECT_INTERNALS_HELPER(Type, ParentType)                                                                \
	  public:                                                                                                         \
		CZ_DELETE_COPY_AND_MOVE(Type);                                                                                \
		Type(const this_is_private& dummy)                                                                            \
			: ParentType(dummy)                                                                                       \
		{                                                                                                             \
		}                                                                                                             \
		using Super = ParentType;                                                                                     \
		                                                                                                              \
	  protected:                                                                                                      \
		inline static Type##Class ms_class;                                                                           \
	  private:                                                                                                        \
	  MLGE_OBJECT_HELPER_FRIENDS(Type)                                                                                \
		static ObjectPtr<Type> allocObject()                                                                          \
		{                                                                                                             \
			if constexpr(!std::is_abstract_v<Type>)                                                                   \
			{                                                                                                         \
				ObjectPtr<Type> obj = allocObjectHelper<Type>();                                                      \
				obj->m_class = &Type::ms_class;                                                                       \
				obj->m_objectName = std::format("{}_{}", obj->m_class->getName(), obj->m_class->getAndIncCounter());  \
				return obj;                                                                                           \
			}                                                                                                         \
			else                                                                                                      \
			{                                                                                                         \
				CZ_LOG(Fatal, "Can't instantiate abstract object");                                                   \
				return nullptr;                                                                                       \
			}                                                                                                         \
		}


/**
 * This should be put on the first line inside the class
 * Use this for mlge objects that are not abstract.
 * If the class is abstract, use MLGE_OBJECT_INTERNALS_ABSTRACT
 */
#define MLGE_OBJECT_INTERNALS(Type, ParentType)     \
	MLGE_OBJECT_INTERNALS_HELPER(Type, ParentType)  \
	MLGE_OBJECT_INTERNALS_HELPER_ABSTRACT_NO(Type)

/**
 * This should be put on the first line inside the class
 * Use this for mlge objects that are abstract 
 */
#define MLGE_OBJECT_INTERNALS_ABSTRACT(Type, ParentType) \
	MLGE_OBJECT_INTERNALS_HELPER(Type, ParentType)       \
	MLGE_OBJECT_INTERNALS_HELPER_ABSTRACT_YES(Type)

/**
 * Put this right after the finishing the class declaration (after the closing }; )
 */
#define MLGE_OBJECT_END(Type)                                                 \
	                                                                          \
	inline void Type##Class::init()                                           \
	{                                                                         \
		m_isAbstract = std::is_abstract_v<Type>;                              \
	}                                                                         \
                                                                              \
	inline ObjectPtr<MObject> Type##Class::createObject()                     \
	{                                                                         \
		if constexpr(!std::is_abstract_v<Type>)                               \
		{                                                                     \
			return mlge::createObject<Type>();                                \
		}                                                                     \
		else                                                                  \
		{                                                                     \
			CZ_VERIFY_F(false, "Object of types {} are abstract.", m_name);   \
			return nullptr;                                                   \
		}                                                                     \
	}

/**
 * Allocates and initializes an object
 * 
 */
template<typename T, typename... Args>
ObjectPtr<T> createObject(Args&& ... args)
{
	static_assert(!std::is_abstract_v<T>, "Object type is abstract.");
	if constexpr(std::is_abstract_v<T>)
	{
		using Testing = decltype(new T(MObject::this_is_private{0}));
	}

	auto obj = T::allocObject();

	// First we call defaultDestruct, then construct if there are any construct parameters
	bool res = obj->preConstruct();
	if (res)
	{
		if constexpr(sizeof...(Args) > 0)
		{
			res = obj->construct(std::forward<Args>(args)...);
		}
	}

	if (res)
	{
		obj->postConstruct();
		return obj;
	}
	else
	{
		CZ_LOG(Error, "Failed to initialize object {}.", obj->getObjectName());
		return nullptr;
	}
}

/**
 * Performs a dynamic cast of an MObject.
 * Returns the object it casted to, or nullptr if the cast could not be performed.
 */
template<typename To, typename From>
To* dynamicCast(From* obj)
{
	static_assert(std::is_base_of_v<MObject, From>, "dynamicCast can only be used on MObject (or derived) classes");

	// If "To" is a base class of "From", then we can use implicit cast
	if constexpr(std::is_base_of_v<To, From>)
	{
		return obj;
	}
	else
	{
		Class& c = Class::get<To>();
		if (obj && c.isBaseOf(obj->getClass()))
		{
			return static_cast<To*>(obj);
		}
		else
		{
			return nullptr;
		}
	}

}

template<class T, class U>
ObjectPtr<T> static_pointer_cast(const ObjectPtr<U>& other) noexcept
{
	return ObjectPtr<T>(static_cast<T*>(other.get()));
}

template<class T, class U>
ObjectPtr<T> static_pointer_cast(ObjectPtr<U>&& other) noexcept
{
	ObjectPtr<U> other_(std::move(other));
	return ObjectPtr<T>(static_cast<T*>(other_.get()));
}

template<class T, class U>
ObjectPtr<T> dynamic_pointer_cast(const ObjectPtr<U>& other) noexcept
{
	return ObjectPtr<T>(dynamicCast<T>(other.get()));
}

template<class T, class U>
ObjectPtr<T> dynamic_pointer_cast(ObjectPtr<U>&& other) noexcept
{
	const auto ptr = dynamicCast<T>(other.get());

	if (ptr)
	{
		ObjectPtr<U> other_(std::move(other));
		return ObjectPtr<T>(ptr);
	}
	else
	{
		return {};
	}
}


} // namespace mlge

