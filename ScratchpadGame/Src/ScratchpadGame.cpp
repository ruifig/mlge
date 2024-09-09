#include "ScratchpadGame.h"

#include "mlge/Experimental/DebugText.h"

#include "PlayerShip.h"
#include "Stats.h"

#include "crazygaze/core/CommandLine.h"

using namespace mlge;
using namespace std::literals::chrono_literals;


MLGE_IMPLEMENT_GAME(ScratchpadGame, "ScratchpadGame");

ScratchpadGame::ScratchpadGame()
	: Game("ScratchpadGame")
{
}

ScratchpadGame::~ScratchpadGame()
{
}

bool ScratchpadGame::init()
{
	if (!Super::init())
	{
		return false;
	}

	m_stats = getLevel().addNewActor<AStats>();
	m_stats.lock()->setPosition({+10,0});

	m_debugText = getLevel().addNewActor<ADebugText>();
	m_debugText.lock()->setPosition({300,0});
	addDebugText("Test debug message");

	m_playerShip = getLevel().addNewActor<APlayerShip>();
	m_playerShip.lock()->setPosition({400, 400});

	m_bkgColour = Color(0x0A, 0x11, 0x72, 255);

	return true;
}

void ScratchpadGame::tick(float deltaSeconds)
{
	Super::tick(deltaSeconds);
}

float ScratchpadGame::startShutdown()
{
	Super::startShutdown();
	return 10.0f;
}

bool ScratchpadGame::isShutdownFinished()
{
	return true;
}

void ScratchpadGame::shutdown()
{
	Super::shutdown();
}

