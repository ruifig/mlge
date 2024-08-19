#include "mlge/Resource/Resource.h"
#include "mlge/Profiler.h"
#include "mlge/Game.h"

#include "crazygaze/core/Logging.h"
#include "crazygaze/core/CommandLine.h"

namespace mlge
{

bool MResource::construct(const MResourceDefinition& definition)
{
	m_definition = &definition;
	return Super::defaultConstruct();
}

MResourceDefinition::~MResourceDefinition()
{
	// At this point, the resource shouldn't be loaded
	CZ_CHECK(m_resource.expired());
}

ObjectPtr<MResource> MResourceDefinition::getResource() const
{
	// If it's already loaded, return it
	if ( auto resource = m_resource.lock())
	{
		return resource;
	}

	CZ_LOG(Log, "Loading resource {} from file \"{}\".", name, narrow(file.native()));
	ObjectPtr<MResource> resource = create();

	if (resource)
	{
		m_resource = resource;
		return resource;
	}
	else
	{
		CZ_LOG(Error, "Failed to load resource {}", name);
		return nullptr;
	}
}

MResourceDefinition::BaseRef::BaseRef()
{
	ResourceManager::get().addDefinitionRef(this);
}

MResourceDefinition::BaseRef::~BaseRef()
{
	ResourceManager::get().removeDefinitionRef(this);
}


//////////////////////////////////////////////////////////////////////////
//		ResourceManager
//////////////////////////////////////////////////////////////////////////

ResourceManager::~ResourceManager()
{
	m_all.refs.clear();
	m_all.definitions.clear();
}

bool ResourceManager::loadDefinitions(ResourceRoot& root, Group& group, const json& jdefs) const
{
	CZ_LOG(Log, "Loading resource definitions");

	int errorCount = 0;

	for(auto j : jdefs)
	{
		try
		{
			std::string type;
			j.at("type").get_to(type);

			std::string className = type + "Definition";

			if (Class* typeClass = Class::find(className.c_str()))
			{
				CZ_CHECK_F(Class::get<MResourceDefinition>().isBaseOf(*typeClass), "Class `{}` is not derived from `MResourceDefinition`", typeClass->getName());

				ObjectPtr<MResourceDefinition> def = static_pointer_cast<MResourceDefinition>(typeClass->createObject());
				def->construct(root);
				def->from_json(j);
				if (group.definitions.find(def->name) != group.definitions.end())
				{
					CZ_LOG(Warning, "Resource '{}' already defined. Being replaced with override.", def->name);
				}
				group.definitions[def->name] = std::move(def);
			}
			else
			{
				errorCount++;
				CZ_LOG(Error, "Resource type '{}' not registered.", type);
			}
		}
		catch(json::exception& ex)
		{
			errorCount++;
			CZ_LOG(Error, "Error reading resource definition. ec={}", ex.what());
		}
	}

	// Resolve all the references created from loading this group
	for(MResourceDefinition::BaseRef* ref : group.refs)
	{
		// Ignore references that don't have the name set
		if (ref->name.size() == 0)
		{
			continue;
		}

		// Firs we try to find it in the definitions we created right now.
		// If not found, then we look in the pre-existing ones.
		ref->def = group.find(ref->name);
		if (ref->def == nullptr)
		{
			ref->def = m_all.find(ref->name);
		}

		if (ref->def == nullptr)
		{
			errorCount++;
			CZ_LOG(Error, "Reference to '{}' could not be resolved.", ref->name);
		}

		const Class& requiredDefClass = ref->getDefClass();
		if (!requiredDefClass.isBaseOf(ref->def->getClass()))
		{
			errorCount++;
			CZ_LOG(
				Error, "Reference to '{}' expected a definition of type '{}', but found one of type '{}'", ref->name,
				requiredDefClass.getName(), ref->def->getClass().getName());
			ref->def = nullptr;
		}
	}

	if (errorCount == 0)
	{
		CZ_LOG(Log, "Finished loading resource definitions");
		return true;
	}
	else
	{
		CZ_LOG(Error, "{} errors found loading resource definitions. Ignoring entire group.", errorCount);
		return false;
	}
}

void ResourceManager::addGroup(Group& group)
{
	// Add definitions to the main group
	for(auto&& def : group.definitions)
	{
		// Overwrite definitions, but warn when it happens
		auto res = m_all.definitions.emplace(def.second->name, std::move(def.second));
		if (!res.second)
		{
			CZ_LOG(
				Warning, "Resource '{}' already exists. Replacing with new one (of type `{}`).", res.first->second->name,
				res.first->second->getClass().getName());
		}
	}
	group.definitions.clear();

	// Re-resolve all pre-existing references.
	// The ones coming from the new group were already resolved
	// Note that new definitions can replace existing ones, but once a definition named e.g "Foo" is created, 
	// there will always be a "Foo" definition. What can happen is that "Foo" can be replaced by another
	// with the same name.
	// This means that point references are guaranteed to resolve.
	for(auto&& ref : m_all.refs)
	{
		ref->def = m_all.find(ref->name);
		CZ_CHECK(ref->def);
	}

	// Add all new references to the main group.
	// THere were already resolved.
	for(auto&& ref : group.refs)
	{
		m_all.refs.emplace(ref);
	}
	group.refs.clear();
}

const MResourceDefinition* ResourceManager::Group::find(std::string_view name) const
{
	auto it = definitions.find(name);
	if (it == definitions.end())
	{
		CZ_LOG(Error, "Resource '{}' not found", name);
		return nullptr;
		
	}
	else
	{
		return it->second.get();
	}
}

bool ResourceManager::loadDefinitions(const fs::path& definitionsFile)
{
	json data;

	std::unique_ptr<ResourceRoot> root;

	try
	{
		fs::path filename = convertToAbsolutePath(definitionsFile);
		std::ifstream f(filename);
		data = json::parse(f);

		root = std::make_unique<ResourceRoot>();
		root->path = filename.parent_path();
	}
	catch(std::exception& ex)
	{
		CZ_LOG(Error, "Error reading or parsing resources json. ec={}", ex.what());
		return false;
	}

	for(auto it = data.begin(); it!=data.end(); it++)
	{
		if (it.key() == "resources" && it->is_array())
		{
			Group group;
			m_currentlyLoadingGroup = &group;
			if (loadDefinitions(*root, group, it.value()))
			{
				addGroup(group);
			}
			m_currentlyLoadingGroup = nullptr;
		}
	}

	m_roots.push_back(std::move(root));
	return true;
}

bool ResourceManager::loadDefinitions()
{
	MLGE_PROFILE_SCOPE(mlge_ResourceManager_loadDefinitions);

	bool res = true;
	res = loadDefinitions(getEnginePath() / "Assets" / "mlge.assets.json");
	if (!res)
	{
		return false;
	}

	res = loadDefinitions(getGamePath() / "Assets" / (std::string(getGameFolderName()) + ".assets.json"));
	if (!res)
	{
		return false;
	}

	int errorCount = 0;
	bool testResources = CommandLine::get().has("TestResources");

	if (testResources)
	{
		for(auto&& def : m_all.definitions)
		{
			ObjectPtr<MResource> resource = def.second->getResource();
			if (!resource)
			{
				errorCount++;
			}
		}
	}

	if (!BaseStaticResourceRef::resolveAll(testResources))
	{
		errorCount++;
	}

	return errorCount == 0 ? true : false;
}

const MResourceDefinition* ResourceManager::findDefinition(std::string_view name) const
{
	return m_all.find(name);
}


void ResourceManager::addDefinitionRef(MResourceDefinition::BaseRef* ref)
{
	// When adding a definition reference, we always add it to the currently loading group.
	CZ_CHECK(m_currentlyLoadingGroup);
	m_currentlyLoadingGroup->refs.emplace(ref);
}

void ResourceManager::removeDefinitionRef(MResourceDefinition::BaseRef* ref)
{
	// When removing a reference, we don't know in what group it is now.
	// - If loading of a definition file failed, then any references being destroyed because they were
	// part of the loading will be in m_currentlyLoadingGroup.
	// - A reference being destroyed because the owing definition was replaced by an overwrite will
	// already be in the m_all.
	if (m_currentlyLoadingGroup)
	{
		m_currentlyLoadingGroup->refs.erase(ref);
	}
	m_all.refs.erase(ref);
}

} // namespace mlge

