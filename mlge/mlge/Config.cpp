#include "mlge/Config.h"
#include "mlge/Game.h"
#include "mlge/Paths.h"

namespace mlge
{

bool Config::init()
{
	// Default engine
	{
		auto inifile = std::make_unique<IniFile>();
		fs::path path = getEnginePath() / "Config" / "mlge.ini";
		if (!inifile->open(path))
		{
			return false;
		}

		m_all.emplace_back(Type::Engine, true, path, std::move(inifile));
	}

	// Already existing in the Bin folder
	{
		auto inifile = std::make_unique<IniFile>();
		fs::path path = getProcessPath() / "mlge.ini";
		if (!inifile->try_open(path))
		{
			// set to empty inifile
			inifile = std::make_unique<IniFile>();
		}

		m_all.emplace_back(Type::Engine, false, path, std::move(inifile));
	}

	return loadGameConfig();
}

bool Config::loadGameConfig()
{
	// Default game config
	{
		auto inifile = std::make_unique<IniFile>();
		fs::path path = getGamePath() / "Config" / (std::string(getGameFolderName()) + ".ini");
		if (!inifile->open(path))
		{
			return false;
		}
		m_all.emplace_back(Type::Game, true, path, std::move(inifile));
	}

	// Already existing in the Bin folder
	{
		auto inifile = std::make_unique<IniFile>();
		fs::path path = getProcessPath() / (std::string(getGameFolderName()) + ".ini");

		if (!inifile->try_open(path))
		{
			// set to empty inifile
			inifile = std::make_unique<IniFile>();
		}

		m_all.emplace_back(Type::Game, false, path, std::move(inifile));
	}

	return true;
}

bool Config::unloadGameConfig()
{
	erase_if(m_all, [](const Set& el)
	{
		return el.type==Type::Game;
	});
	
	return true;
}

} // namespace mlge
