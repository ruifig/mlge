#include "mlge/Paths.h"

// This is implemented with the MLGE_IMPLEMENT_GAME
mlge::fs::path getGameRelativePath();

namespace mlge
{

fs::path getBuildRoot()
{
	return MLGE_BUILD_ROOT;
}

fs::path getRootPath()
{
	return convertToAbsolutePath("./");
}

fs::path getEnginePath()
{
	// Calculate the engine's files folder relative to the repository root folder
	// We do this by using the knowledge where __FILE__ is relative to MLGE_BUILD_ROOT
	auto engineRelativePath = fs::relative(fs::path(__FILE__).parent_path().parent_path(),  getBuildRoot());
	return convertToAbsolutePath(getRootPath() / engineRelativePath);
}

fs::path getGamePath()
{
	return convertToAbsolutePath(getRootPath() / getGameRelativePath());
}

fs::path convertToAbsolutePath(const fs::path& path)
{
	fs::path res;

	if (path.is_relative())
	{
		res = getProcessPath() / ".." / path;
	}
	else
	{
		res = path;
	}

	res = fs::absolute(res);
	return res;
}

} // namespace mlge

