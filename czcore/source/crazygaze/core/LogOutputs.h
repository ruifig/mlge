#pragma once

#include "Logging.h"
#include "Singleton.h"
#include "AsyncCommandQueue.h"
#include "Semaphore.h"

namespace cz
{

class LogOutputs : public Singleton<LogOutputs>
{
  public:
	
	LogOutputs();

	using LogFunc = std::function<void(LogLevel level, const char* category, const char* timestamp, const char* msg)>;

	void add(void* tag, LogFunc&& logFunc);
	void remove(void* tag);
	void log(bool debuggerOutput, LogLevel level, const char* category, const char* timestamp, const char* msg);

  protected:

	std::vector<std::pair<void*, LogFunc>> m_outputs;
};


class FileLogOutput
{
  public: 
	FileLogOutput() = default;
	~FileLogOutput();

	bool open(const std::string& directory, const std::string& filename);

  private:
	void logMsg(LogLevel level, const char* category, const char* timestamp, const char* msg);

	void flush();

	AsyncCommandQueueAutomatic m_q;
	std::ofstream m_file;
	std::string m_filename;
	Semaphore m_finish;
};

} // namespace cz

