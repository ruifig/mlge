#pragma once

#include "crazygaze/core/IniFile.h"
#include "crazygaze/core/Singleton.h"
#include "crazygaze/core/CommandLine.h"

namespace mlge
{

/**
 * Config allows searching through all the ini files and command line.
 *
 * Config files are loaded in the following order:
 * - Original engine settings
 * - Saved engine settings
 * - Original game settings
 * - Saved game settings
 *
 *
 * When querying for a value, it first checks the command line, then the list above in the  reverse order, This for example the
 * saved game settings to override the original game settings.
 */
class Config : public Singleton<Config>
{
public:

	bool init();
	bool loadGameConfig();
	bool unloadGameConfig();


	/**
	 * Retrieves the specified value, or the default value if it doesn't exist or cannot be converted to the desired type.
	 */
	template<typename T>
	T getValueOrDefault(const char* section, const char* key, T defaultValue) const
	{
		T res;

		// Command line overrides anything, so we search that first
		if (CommandLine::get().getValue((std::string("[") + section + "]" + key).c_str(), res))
		{
			return res;
		}

		// Search starting from the end, so we search in the following order:
		// - Saved Game settings
		// - Original game settings
		// - Saved Engine settings
		// - Original Engine settings
		for(auto it = m_all.rbegin(); it!=m_all.rend(); ++it)
		{
			if (it->inifile->getValue<T>(section, key, res))
			{
				return res;
			}
		}

		return defaultValue;
	}

	template<typename T>
	void setGameValue(const char* section, const char* key, T value)
	{
		Set* set = getSet(Type::Game, false);
		if (!set)
		{
			return;
		}

		set->inifile->setValue(section, key, value);
	}

	/**
	 * Saves the non-default files
	 */
	void save()
	{
		for(auto&& set : m_all)
		{
			if (!set.defaultSet)
			{
				set.inifile->save(set.path);
			}
		}
	}

protected:

	enum class Type
	{
		Engine,
		Game
	};

	struct Set
	{
		Type type;
		bool defaultSet;
		fs::path path;
		std::unique_ptr<IniFile> inifile;
	};

	Set* getSet(Type type, bool defaultSet)
	{
		for(auto&& set : m_all)
		{
			if (set.type == type && set.defaultSet == defaultSet)
			{
				return &set;
			}
		}

		return nullptr;
	}


	std::vector<Set> m_all;
};

} // namespace mlge