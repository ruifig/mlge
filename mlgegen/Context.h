#pragma

namespace mlge::gen
{


struct ProcessedFile
{
	std::string name;
	fs::path path;
};

struct Context
{
	std::unordered_map<std::string, std::unique_ptr<ProcessedFile>> files;
};

}

