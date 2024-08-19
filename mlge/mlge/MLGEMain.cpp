#include "mlge/Engine.h"

namespace mlge
{
	void globalConstructorsTouch();
}

int main( int argc, char* argv[])
{
	mlge::globalConstructorsTouch();
	mlge::Engine engine;

	if (!engine.init(argc, argv))
	{
		return EXIT_FAILURE;
	}

	return engine.run() ? EXIT_SUCCESS : EXIT_FAILURE;
}

