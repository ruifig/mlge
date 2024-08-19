# mlge

"mlge" stands for "My Little Game Engine".
At the moment, it's not meant to be a production quality engine, but rather something small that can be used to write samples for other projects.
E.g, you have some SDK/library meant for games, and need to write some minimalistic game samples.

# Requirements

* CMake
* A >=C++20 compiler (Only Visual Studio 2022 works at the moment)

# Integrating with your project

Add mlge to your repository either through copy&paste, or submodules.
Although other folder structures are possible the following is recommended:

```
<YourRepoRoot>
|   CMakeLists.txt             - Your game's CMakeList.txt
|
+---Bin                            - Will be created by mlge's cmake scripts
+---Build                          - Your cmake build folder
+---Deps                           - Where you should put your own dependencies
+---mlge                           - The entire mlge's repository contents (e.g as a submodule)
\---YourGame                       - Your game root folder
    |   CMakeLists.txt             - Your game's CMakeList.txt
    |
    +---Assets
    |   YourGame.assets.json   - This must exist and defines the assets for your game 
    |
    +---Config
    |       YourGame.ini           - This must exist, but can be empty
    |
    \---Src                        - Your game's source code
```

Considering your game is called "MyGame", from your root CMakeLists.txt, do the following:

```
# Include this to expose anything mlge needs in the global scope
include(mlge/CMake/presetup.cmake)

# define the mlge cmake target(s)
add_subdirectory(./mlge)

# define your game's cmake target(s)
add_subdirectory(./YourGame)
```

From your game's CMakeLists.txt, do the following:

```

... do the add_executable(YourGame ...) and any other cmake things you need ...

# Add your dependencies, including mlge
target_link_libraries(YourGame PRIVATE
	mlge
    ... other dependencies ...
)

# REQUIRED: Setup some things mlge needs for your game to work with the engine
mlge_setupBinaryTarget(YourGame)

```

## Available CMake options

* `MLGE_SAMPLES` - ON/OFF (`OFF` by default). If set to `ON`, it will generate targets for the mlge samples
* `MLGE_TESTS` - ON/OFF (`OFF` by default). If set to `ON`, it will generate targets for the mlge unit tests

