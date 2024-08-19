#include "mlge/Root.h"

#include "mlge/Resource/Resource.h"
#include "mlge/Render/Renderer.h"
#include "mlge/Render/RenderQueue.h"
#include "mlge/Profiler.h"
#include "mlge/Config.h"

#include "crazygaze/core/CommandLine.h"
#include "crazygaze/core/Logging.h"
#include "crazygaze/core/LogOutputs.h"

#if MLGE_EDITOR
	#include "mlge/Editor/Editor.h"
#endif

namespace mlge
{

struct RootImpl : public Root
{
	// This needs to be the first one, so that the other singletons can query the command line.
	LogOutputs logOutputs;
	CommandLine cmdLine;
	FileLogOutput fileLogOutput;
	Config config;
	Profiler profiler;
	Renderer renderer;
	ResourceManager resourceManager;
	RenderQueue renderQueue;
#if MLGE_EDITOR
	// Even for an editor build, we use a unique_ptr, so it only gets initialized if `-game` is not specified in the command line
	std::unique_ptr<editor::Editor> editor;
#endif

	virtual bool init() override
	{
		fileLogOutput.open("", std::string(getGameFolderName()));

		// This needs to be the first one to be initialized, so the other singletons can query the config
		// NOTE: Commandline is initialized before this, outside of Root
		if (!config.init())
		{
			return false;
		}
		
		profiler.init();

		if (!renderer.init())
		{
			return false;
		}

		if (!resourceManager.init())
		{
			return false;
		}

		if (!renderQueue.init())
		{
			return false;
		}

#if MLGE_EDITOR
		gIsGame = CommandLine::get().has("game") == nullptr ? false : true;
		if (!gIsGame)
		{
			editor = std::make_unique<editor::Editor>();
			if (!editor->init())
			{
				return false;
			}
		}
#endif

		return true;
	}
};

std::unique_ptr<Root> Root::create()
{
	return std::make_unique<RootImpl>();
}

} // namespace mlge

