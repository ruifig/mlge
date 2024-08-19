#include "mlge/Profiler.h"
#include "crazygaze/core/Logging.h"

namespace mlge
{

Profiler::Profiler()
{
}

Profiler::~Profiler()
{
#if MLGE_PROFILER_ENABLED
	if (rmt)
	{
		rmt_DestroyGlobalInstance((Remotery*)rmt);
		// For performance reasons, rmt is a static, so we need to reset it, otherwise if the client calls
		// ludeo_Initialize again, this would be garbage.
		rmt = nullptr;
	}
#endif
}

bool Profiler::init()
{
#if MLGE_PROFILER_ENABLED
	Remotery* ptr;
	rmtSettings* settings = rmt_Settings();
	settings->port = 17815; 
	settings->reuse_open_port = true;
	settings->limit_connections_to_localhost = true;
	settings->enableThreadSampler = true;
	settings->msSleepBetweenServerUpdates = 4;
	settings->messageQueueSizeInBytes = 1024 * 1024;
	settings->maxNbMessagesPerUpdate = 1000;

	rmtError error = rmt_CreateGlobalInstance(&ptr);
	if (error != RMT_ERROR_NONE)
	{
		CZ_LOG(Error, "Error launching profiler (Remotery). Error={}", static_cast<int>(error));
		return false;
	}
	rmt = ptr;
#endif

	return true;
}

 }	// namespace mlge

