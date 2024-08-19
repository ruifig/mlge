#include "mlgeSample.h"

#include "crazygaze/core/CommandLine.h"

using namespace mlge;
using namespace std::literals::chrono_literals;

MLGE_IMPLEMENT_GAME(Sample, "mlgeSample");

Sample::Sample()
	: Game("Sample")
{
}

Sample::~Sample()
{
}

bool Sample::init()
{
	if (!Super::init())
	{
		return false;
	}

	m_bkgColour = Color(0x0A, 0x11, 0x72, 255);

	return true;
}

void Sample::tick(float deltaSeconds)
{
	Super::tick(deltaSeconds);
}

float Sample::startShutdown()
{
	Super::startShutdown();
	return 10.0f;
}

bool Sample::isShutdownFinished()
{
	return true;
}

void Sample::shutdown()
{
	Super::shutdown();
}


