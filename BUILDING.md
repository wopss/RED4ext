# Building

The library can only be built on Windows. Below are the recommended steps to build it:

1. Download and install [Visual Studio 2022 Community Edition](https://www.visualstudio.com/) or a newer version. Make sure to check "desktop development for c++" in installer
2. Download and install [CMake 3.23](https://cmake.org/) or a newer version.
3. Clone this repository.
4. Clone the dependencies (`git submodule update --init --recursive`). (Install git if needed) Use 'Developer Command Prompt for VS' to do this.
5. Create and go to the `build` directory and run `cmake ..`. Use 'Developer Command Prompt for VS' to do this.
6. Open the solution (`build/RED4ext.slnx`).
7. Build the projects.
