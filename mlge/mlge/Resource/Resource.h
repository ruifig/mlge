#pragma once

#include "mlge/Common.h"
#include "mlge/Misc/LinkedList.h"
#include "mlge/Object.h"
#include "mlge/Paths.h"

#include "crazygaze/core/Logging.h"
#include "crazygaze/core/Singleton.h"
#include "crazygaze/core/StringUtils.h"

#if MLGE_EDITOR
	#include "mlge/Editor/ResourceWindow.h"
#endif

// #RVF : Remove this if not used
#if 0
/**
 * These are based on what nlohmann json provides. E.g: NLOHMANN_DEFINE_TYPE_INTRUSIVE
 */

// Define json for base struct
#define JSON_DEFINE_TYPE_INTRUSIVE(Type, ...)                                           \
	friend void to_json(nlohmann::json& nlohmann_json_j, const Type& nlohmann_json_t)   \
	{                                                                                   \
		NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_TO, __VA_ARGS__))        \
	}                                                                                   \
	friend void from_json(const nlohmann::json& nlohmann_json_j, Type& nlohmann_json_t) \
	{                                                                                   \
		NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, __VA_ARGS__))      \
	}

// Define json for derived struct
#define JSON_DEFINE_TYPE_INTRUSIVE_DERIVED(Type, Super, ...)                            \
	friend void to_json(nlohmann::json& nlohmann_json_j, const Type& nlohmann_json_t)   \
	{                                                                                   \
		to_json(nlohmann_json_j, static_cast<const Super&>(nlohmann_json_t));           \
		NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_TO, __VA_ARGS__))        \
	}                                                                                   \
	friend void from_json(const nlohmann::json& nlohmann_json_j, Type& nlohmann_json_t) \
	{                                                                                   \
		from_json(nlohmann_json_j, static_cast<Super&>(nlohmann_json_t));               \
		NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, __VA_ARGS__))      \
	}
#endif

namespace mlge
{

struct ResourceRoot
{
	fs::path path;
};

class MResource;
class ResourceManager;


MLGE_OBJECT_START(MResourceDefinition, MObject, "Base class for all resource definitions.\nA resource definition specifies the parameters for a resource creation." )

/**
 * Base class for all resource definitions
 * A resource definition puts together all the information about a particular resource, but doesn't actually hold the actual
 * resource data.
 * E.g: It contains the resource name and file to load it from, but not the actual file data.
 * When a game loads, it loads all the resource definitions, so it can find and load resources on demand.
 */
class MResourceDefinition : public MObject
{
	MLGE_OBJECT_INTERNALS_ABSTRACT(MResourceDefinition, MObject)

  public:

	~MResourceDefinition();

	virtual bool construct(const ResourceRoot& root)
	{
		m_root = &root;
		return Super::defaultConstruct();
	}

	#if MLGE_EDITOR
	virtual std::unique_ptr<editor::BaseResourceWindow> createEditWindow() = 0;
	#endif

	/**
	 * Base class for the Ref<T> template
	 */
	struct BaseRef
	{
		BaseRef();
		virtual ~BaseRef();

		std::string name;

		/**
		 * Once all definitions are loaded, the resource manager iterates through all DefinitionRef instances and
		 * sets this field.
		 * A Definition Ref that fails to resolve is considered a fatal error.
		 */
		const MResourceDefinition* def = nullptr;

		virtual Class& getDefClass() const = 0;
	};

	/**
	 * Reference to a resource definition.
	 * Definitions use this for when they depend on another resource.
	 */
	template<typename TDef>
	struct Ref : public BaseRef
	{

		virtual Class& getDefClass() const override
		{
			return Class::get<TDef>();
		}

		const TDef* get() const
		{
			return dynamicCast<const TDef>(def);
		}
	};

	const ResourceRoot& getRoot() const
	{
		CZ_CHECK(m_root);
		return *m_root;
	}

	/**
	 * The resource name.
	 */
	std::string name;

	/**
	 * The resource's filename. This is the file the resource will be loaded from
	 */
	fs::path file;

	const std::string_view getTypeName() const
	{
		std::string_view className = m_class->getName();
		return std::string_view(className.begin(), className.end() - (int)strlen("Definition"));
	}

	/**
	 * Tells if the resource this definition refers to is currently loaded
	 */
	bool isLoaded() const
	{
		return m_resource.expired() ? false : false;
	}

	/**
	 * Returns the resource.
	 * If the resource is not loaded, it will be loaded.
	 * If loading fails, it returns nullptr.
	 */
	ObjectPtr<MResource> getResource() const;

  protected:
	/**
	 * Specifies a root for this definition. E.g: Root folder to prefix to file name.
	 */
	const ResourceRoot* m_root = nullptr;

	friend ResourceManager;

	virtual void to_json(nlohmann::json& j) const
	{
		j["type"] = getTypeName();
		j["name"] = name;
		if (file.native().size())
		{
			j["file"] = file;
		}
	}

	virtual void from_json(const nlohmann::json& j)
	{
		j.at("name").get_to(name);
		if (j.count("file") !=0)
		{
			j.at("file").get_to(file);
		}
	}

	/**
	 * Creates the resource from the definition
	 * Derived classes need to implement this to create the right resource
	 */
	virtual ObjectPtr<MResource> create() const = 0;

	/**
	 * The resource created from this definition.
	 * It's a weak ptr, so it's up to the game to control what resources it wants to keep loaded
	 */
	mutable WeakObjectPtr<MResource> m_resource;
};
MLGE_OBJECT_END(MResourceDefinition)


MLGE_OBJECT_START(MResource, MObject, "Base class for resources")

/**
 * Base class for all resources
 * For each class derived from MResource, there needs to be respective class derived from MResourceRef
 */
class MResource : public MObject
{
	MLGE_OBJECT_INTERNALS(MResource, MObject)

  public:

	virtual bool construct(const MResourceDefinition& definition);

	const MResourceDefinition* getDefinition() const
	{
		return m_definition;
	}

  protected:

	/**
	 * If this resource was created from a definition, this points to the definition
	 */
	const MResourceDefinition* m_definition = nullptr;
};
MLGE_OBJECT_END(MResource)	

class ResourceManager : public Singleton<ResourceManager>
{
  public:

	~ResourceManager();

	bool init() { return true; }

	/**
	 * Loads a json file with resource definitions
	 *
	 * @param definitionsFile
	 *	Assets definition file to load. It can be an absolute or relative path.
	 *	If a relative path, it will be assumed as a path relatives to <ProcessDir>\..\<GameFolderName>\ .
	 */
	bool loadDefinitions(const fs::path& definitionsFile);

	/**
	 * Tries to load the game's default assets definition file, which is:
	 * <ProcessDir>\..\<GameFolderName>\<GameFolderName>.assets.json
	 *
	 * The engine calls this automatically. There is no need for the game to call this
	 */
	bool loadDefinitions();

	/**
	 * Find a resource definition
	 * Returns the resource definition if found or nullptr if not found
	 */
	const MResourceDefinition* findDefinition(std::string_view name) const;


	std::vector<ObjectPtr<MResourceDefinition>> getAllDefinitions()
	{
		std::vector<ObjectPtr<MResourceDefinition>> res;
		res.reserve(m_all.definitions.size());
		for(auto&& p : m_all.definitions)
		{
			res.push_back(p.second);
		}
		return res;
	}


  protected:

	friend MResourceDefinition::BaseRef;
	void addDefinitionRef(MResourceDefinition::BaseRef* ref);
	void removeDefinitionRef(MResourceDefinition::BaseRef* ref);

  private:

	/**
	 * Assets are loaded in groups.
	 * If a group loads successfully, then it is added to the main group.
	 * 
	 * That's why when processing a resource definition file, we first add those definitions to separate containers.
	 * Once we figure out that there are no problems, those definitions are then added to the main group.
	 */
	struct Group
	{
		std::unordered_map<std::string_view, ObjectPtr<MResourceDefinition>> definitions;
		std::set<MResourceDefinition::BaseRef*> refs;
		const MResourceDefinition* find(std::string_view name) const;
	};

	bool loadDefinitions(ResourceRoot& root, Group& group, const json& jdefs) const;
	void addGroup(Group& group);

	std::vector<std::unique_ptr<ResourceRoot>> m_roots;
	Group m_all;
	Group* m_currentlyLoadingGroup = nullptr; 
};

/**
 * Base class for StaticResourceRef<T>
 */
class BaseStaticResourceRef : public DoublyLinked<BaseStaticResourceRef>
{
  public:
	BaseStaticResourceRef(const char* resourceName)
		: m_resourceName(resourceName)
	{
		ms_all.pushBack(this);
	}

	virtual ~BaseStaticResourceRef()
	{
		ms_all.remove(this);
	}

	/**
	 * Used internally to resolve all references
	 */
	static bool resolveAll(bool testLoadResource = false)
	{
		CZ_LOG(Log, "Resolving all StaticResourceRef instance...");
		int errorCount = 0;
		int count = 0;
		for(auto&& ref : ms_all)
		{
			count++;
			CZ_LOG(Verbose, "Resolving '{}'", ref->m_resourceName);

			ref->m_def = ResourceManager::get().findDefinition(ref->m_resourceName);

			if (ref->m_def)
			{
				if (testLoadResource)
				{
					if (!ref->testLoad())
					{
						errorCount++;
					}
				}
			}
			else
			{
				errorCount++;
			}
		}

		if (errorCount == 0)
		{
			CZ_LOG(Log, "Resolved {} references", count);
		}
		else
		{
			CZ_LOG(Error, "Failed to resolve {} references", errorCount);
		}

		return errorCount == 0 ? true : false;
	}

  protected:

	template<typename T>
	friend class ResourceRef;

	virtual bool testLoad() = 0;

	inline static DoublyLinkedList<BaseStaticResourceRef> ms_all;
	const MResourceDefinition* m_def = nullptr;
	const char* m_resourceName = nullptr;
};

/**
 * A resource reference that is meant to be static.
 * E.g: A class that knows it will be using a specific resource, can add this as a static member, and have the resource validated
 * at launch time. E.g:
 * 
 *	class SomeClass
 *	{
 *		// ....
 *		inline static StaticResourceRef<MTTFFont> ms_fontRef = "fonts/Vera";
 *	};
 *
 * Note that this doesn't in itself doesn't load the resource. It's loaded on demand or for validation during launch time.
 */
template<typename T>
class StaticResourceRef : public BaseStaticResourceRef
{
  public:
	using BaseStaticResourceRef::BaseStaticResourceRef;

	/**
	 * Loads the resources
	 */
	ObjectPtr<T> getResource()
	{
		CZ_CHECK(m_def);
		return dynamic_pointer_cast<T>(m_def->getResource());
	}

  protected:
	virtual bool testLoad() override
	{
		return getResource() ? true : false;
	}
};

/**
 * A resource reference that will automatically load the resource. This can be used by classes to have a resource loaded
 * automatically. E.g:
 *
 *	class SomeClass
 *	{
 *		// ....
 *
 *		// This resource will automatically load when the SomeClass instance is created.
 *		ResourceRef<MTTFFont> ms_fontRef = "fonts/Vera";
 *
 *		// It can also be initialized by pointing to a StaticResourceRef...
 *		inline static StaticResourceRef<MTTFFont> ms_fontRef = "fonts/Vera";
 *		ResourceRef<MTTFFont> ms_fontRef = ms_fontRef;
 *	};
 */
template<typename T>
class  ResourceRef
{
  public:
	ResourceRef(std::string_view name)
	{
		auto def = ResourceManager::get().findDefinition(name);
		CZ_CHECK_F(def, "Failed to resolve resource reference '{}'", name);
		init(def);
	}

	ResourceRef(BaseStaticResourceRef& ref)
	{
		CZ_CHECK(ref.m_def);
		init(ref.m_def);
	}

	T* operator->() const
	{
		return m_ptr.get();
	}

	explicit operator bool() const noexcept
	{
		return m_ptr.operator bool();
	}

	operator const ObjectPtr<T>& () const noexcept
	{
		return m_ptr;
	}

  protected:

	void init(const MResourceDefinition* def)
	{
		m_def = def;
		if (m_def)
		{
			m_ptr = dynamic_pointer_cast<T>(m_def->getResource());
			CZ_CHECK_F(m_ptr, "Failed to load resource'{}'", m_def->name);
		}
	}

	const MResourceDefinition* m_def;
	ObjectPtr<T> m_ptr;
};



} // namespace mlge

