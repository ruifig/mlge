#include "mlgegen_custom.h"
#include "Context.h"

const char* gHelp = R"(
-- mlgegen.exe --
Parses and generates code for mlge.
Parameters:
	-i <Directory> : Directory to look for source files making use of code generation macros.
	-o <Directory> : Directory where to output the generated files.

)";

using namespace cz;

using namespace mlge::gen;

Context gCtx;

void processFile(fs::path file)
{
	CZ_LOG(Log, "Processing file '{}'", narrow(file.c_str()));


}

int mainImpl()
{

	fs::path inputFolder;
	fs::path outputFolder;
	if (!CommandLine::get().getValue("i", inputFolder) || !CommandLine::get().getValue("o", outputFolder))
	{
		CZ_LOG(Error, "Invalid or missing parameters.");
		printf("%s\n", gHelp);
		return EXIT_FAILURE;
	}

	if (!fs::exists(inputFolder) || !fs::is_directory(inputFolder))
	{
		CZ_LOG(Error, "Specified input folder ('{}') doesn't exist or is not a directory.", narrow(inputFolder.c_str()));
		return EXIT_FAILURE;
	}

	if (!fs::exists(outputFolder))
	{
		fs::create_directories(outputFolder);
	}

	//
	// Find all header files
	//
	for(const fs::directory_entry& entry :
		fs::recursive_directory_iterator(
			inputFolder,
			fs::directory_options::follow_directory_symlink
			))
	{
		if (!entry.is_regular_file())
		{
			continue;
		}

		std::string ext = narrow(entry.path().extension().c_str());
		if (!(asciiStrEqualsCi(ext, ".h") || asciiStrEqualsCi(ext, ".hpp")))
		{
			continue;
		}

		//CZ_LOG(Log, "{}, is_directory={}", narrow(entry.path().c_str()), entry.is_directory())
		processFile(entry.path());
	}

	
	mlge::gen::test1();
	return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
	LogOutputs logOutputs;
	CommandLine cmdLine;

	cmdLine.init(argc, argv);

	try
	{
		return mainImpl();
	}
	catch(std::exception& ex)
	{
		CZ_LOG(Fatal, "Exception: {}", ex.what());
		return EXIT_FAILURE;
	}
}


