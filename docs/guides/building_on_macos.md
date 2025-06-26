# Building Nyan-Doom on macOS
This is a basic guide for building Nyan-Doom for a x86_64 or arm64 macOS target using brew. 
## Configure brew
[brew](https://brew.sh) is a package manager for macOS and Linux. we will use it to download everything we need to build Nyan-Doom.

To install it we need to run:
```
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```
On x86_64 machines, brew will be installed in `/usr/local/homebrew`.

On arm64 machines, brew will be installed in `/opt/homebrew`.
## Install Build Dependencies
Install cmake, SDL2 and additional dependencies for Nyan-Doom:
```
brew install cmake pkgconf libopenmpt fluid-synth libvorbis libzip mad portmidi sdl2 sdl2_image sdl2_mixer
```
## Build Nyan-Doom
Make a clone of the Nyan-Doom Git repository:
```
git clone https://github.com/andrikpowell/nyan-doom.git
```
Prepare the build folder, generate the build system, and compile:
```
cd nyan-doom
cmake -Sprboom2 -Bbuild -DCMAKE_BUILD_TYPE=Release -DENABLE_LTO=ON
cmake --build build
```

The newly built binaries are located in the build folder.

## Collect DYLIB Files
Create a release folder next to the build folder and copy the Binaries and .wad files to it:
```
mkdir release
cp ./build/nyan-doom ./release/nyan-doom
cp ./build/*.wad ./release/
```

Install the "dylibbundler" program and use it to bundle the .dylib files:

```
brew install dylibbundler

cd ./release
dylibbundler -od -b -x ./nyan-doom -d ./libs/ -p @executable_path/libs
```

## Final Steps

Since this is a release build, it's customary to remove symbols from the binaries (and since we are changing the binary file, we will need to codesign it again):

```
strip ./nyan-doom
codesign --force --deep --preserve-metadata=entitlements,requirements,flags,runtime --sign - "./nyan-doom"
```
Finally, add the files to an archive with today's date:
```
zip -r ./nyan-doom-$(date +"%Y%m%d")-mac.zip . -x .\*
```
