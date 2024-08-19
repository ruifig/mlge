#pragma once

#include "mlge/Common.h"
#include "crazygaze/core/PlatformUtils.h"

namespace mlge
{

/**
 * Returns the full path of the folder that was used as the root folder when building the engine and game
 */
fs::path getBuildRoot();

/**
 * Returns the root path of an mlge project. This is the folder that contains Bin
 */
fs::path getRootPath();

/**
 * Returns the path to engine path. E.g, the "<ProjectRoot>/mlge" folder.
 */
fs::path getEnginePath();


/**
 * Returns the path to the game folder. E.g, the "<ProjectRoot>/<GameName>" folder.
 */
fs::path getGamePath();

/**
 * Given a file path, if the path is relative, it converts it to a path relative to the project root
 */
fs::path convertToAbsolutePath(const fs::path& path);

}

/**
 * Returns the game's folder name.
 */
std::string_view getGameFolderName();

