# Seele
The Seele engine is intendent to be a general purpose 3D Engine, built from the ground up to enforce verbosity for modern APIs,
but keeping it simple to use, with lots of additional and optional parameters.

## Getting started
The project is built using standard CMake 3.15, so that and a modern C++17 compiler should be the only prerequisites.

Clone the repository and initialize the dependencies with `git submodule update --init --recursive`
~~*SLang library hasn't been included in the automatic build yet, so you have to manually compile that project, found in external/slang*~~ This has been fixed, but not extensively tested


## Linux notes:
All dependencies from glfw are required: `libx11-dev, libxrandr-dev, libxinerama-dev, libxcursor-dev, libxi-dev`
