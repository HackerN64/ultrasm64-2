# Ultra Super Mario 64 2

This repository is a clean slate for basic enhancements built upon the *Super Mario 64* decompilation. It is a spritual successor to CrashOveride95's *UltraSM64*.

## Quick Start (for Ubuntu)

1. Install prerequisites: `sudo apt install -y build-essential git pkgconf python3 isl libgmp-dev libmpfr-dev libmpc-dev`
2. Clone the repo from within Linux: `git clone https://github.com/HackerN64/ultrasm64-2.git`
3. Place a Super Mario 64 ROM called `baserom.<VERSION>.z64` into the project folder for asset extraction, where `VERSION` can be `jp`, `us`, `eu`, `sh`.
4. Run `make` to build. Specify the version through `make VERSION=<VERSION>`. Add `-j4` to improve build speed (hardware dependent).

Ensure the repo path length does not exceed 255 characters. Long path names result in build errors.

## Installation

### Windows

Install WSL and a distro of your choice following
[Windows Subsystem for Linux Installation Guide for Windows 10.](https://docs.microsoft.com/en-us/windows/wsl/install-win10)
We recommend either Debian or Ubuntu 18.04 Linux distributions under WSL.
Note: WSL1 does not currently support Ubuntu 20.04.

Next, clone the SM64 repo from within the Linux shell:
`git clone https://github.com/HackerN64/ultrasm64-2.git`

Then continue following the directions in the [Linux](#linux) installation section below.

### Linux

There are 3 steps to set up a working build.

#### Step 1: Install dependencies

The build system has the following package requirements:
 * pkgconf
 * python3 >= 3.6

##### Required Packages

Dependency installation instructions for common Linux distros are provided below:

###### Fedora / RHEL
To install build dependencies:
```
sudo dnf group install -y development-tools
sudo dnf install -y python pkgconf isl-devel gmp-devel mpfr-devel libmpc-devel
```
For SUSE users, please use `mpc-devel` in place of `libmpc-devel`.

###### Debian / Ubuntu
To install build dependencies:
```
sudo apt install -y build-essential git pkgconf python3 isl libgmp-dev libmpfr-dev libmpc-dev
```

###### Arch Linux
To install build dependencies:
```
sudo pacman -S base-devel python gcc gcc-libs libisl libmpc zstd lib32-gcc-libs glibc libmpc mpfr gmp
```

##### Setup Hackerchain

1. Download the Hackerchain setup script [here](https://gist.github.com/mountainflaw/820121d579b3d8e83194b69d9b287753).
2. Create a directory where you'd like to have the build process and build output take place.
3. After running and building Hackerchain, define the environment variable `HACKERCHAIN` in your shell's init file (`~/.bashrc` for Bash typically), have it point to the `bin` directory inside your directory for building Hackerchain.
4. Restart your terminal.

#### Step 2: Copy baserom(s) for asset extraction

For each version (jp/us/eu/sh) for which you want to build a ROM, put an existing ROM at
`./baserom.<VERSION>.z64` for asset extraction.

##### Step 3: Build the ROM

Run `make` to build the ROM (defaults to `VERSION=us`).
Other examples:
```
make VERSION=jp -j4       # build (J) version instead with 4 jobs
make VERSION=eu COMPARE=0 # build (EU) version but do not compare ROM hashes
```

Resulting artifacts can be found in the `build` directory.

The full list of configurable variables are listed below, with the default being the first listed:

* ``VERSION``: ``jp``, ``us``, ``eu``, ``sh``
* ``GRUCODE``: ``f3d_old``, ``f3d_new``, ``f3dex``, ``f3dex2``, ``f3dzex``
* ``COMPARE``: ``1`` (compare ROM hash), ``0`` (do not compare ROM hash)
* ``NON_MATCHING``: Use functionally equivalent C implementations for non-matchings. Also will avoid instances of undefined behavior.

### macOS

With macOS, you may either use Homebrew or [Docker](#docker-installation).

#### Homebrew

#### Step 1: Install dependencies
Install [Homebrew](https://brew.sh) and the following dependencies:
```
brew update
brew install coreutils make pkg-config tehzz/n64-dev/mips64-elf-binutils
```

#### Step 2: Copy baserom(s) for asset extraction

For each version (jp/us/eu/sh) for which you want to build a ROM, put an existing ROM at
`./baserom.<VERSION>.z64` for asset extraction.

##### Step 3: Build the ROM

Use Homebrew's GNU make because the version included with macOS is too old.

```
gmake VERSION=jp -j4       # build (J) version instead with 4 jobs
```

### Docker Installation

#### Create Docker image

After installing and starting Docker, create the docker image. This only needs to be done once.
```
docker build -t sm64 .
```

#### Build

To build, mount the local filesystem into the Docker container and build the ROM with `docker run sm64 make`.

##### macOS example for (U):
```
docker run --rm --mount type=bind,source="$(pwd)",destination=/sm64 sm64 make VERSION=us -j4
```

##### Linux example for (U):
For a Linux host, Docker needs to be instructed which user should own the output files:
```
docker run --rm --mount type=bind,source="$(pwd)",destination=/sm64 --user $UID:$GID sm64 make VERSION=us -j4
```

Resulting artifacts can be found in the `build` directory.

## Project Structure

	sm64
	├── actors: object behaviors, geo layout, and display lists
	├── asm: handwritten assembly code, rom header
	│   └── non_matchings: asm for non-matching sections
	├── assets: animation and demo data
	│   ├── anims: animation data
	│   └── demos: demo data
	├── bin: C files for ordering display lists and textures
	├── build: output directory
	├── data: behavior scripts, misc. data
	├── doxygen: documentation infrastructure
	├── enhancements: example source modifications
	├── include: header files
	├── levels: level scripts, geo layout, and display lists
	├── lib: SDK library code
	├── rsp: audio and Fast3D RSP assembly code
	├── sound: sequences, sound samples, and sound banks
	├── src: C source code for game
	│   ├── audio: audio code
	│   ├── buffers: stacks, heaps, and task buffers
	│   ├── engine: script processing engines and utils
	│   ├── game: behaviors and rest of game source
	│   ├── goddard: Mario intro screen
	│   └── menu: title screen and file, act, and debug level selection menus
	├── text: dialog, level names, act names
	├── textures: skybox and generic texture data
	└── tools: build tools

## Contributing

Pull requests are welcome. For major changes, please open an issue first to
discuss what you would like to change.

Run `clang-format` on your code to ensure it meets the project's coding standards.

Official Discord: https://discord.gg/brETAakcXr
