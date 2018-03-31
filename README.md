# Sega Genesis/Megadrive tinkering

> A work in progress

Given that a lot of the docs are outdated this may prove to be helpful as a sort of tutorial or set of examples.

## Installation

You will need the latest Gendev to compile the source and RetroArch to run ROMs

- [Gendev - Linux](https://github.com/kubilus1/gendev/releases)
- [Gendev - MacOS](https://github.com/SONIC3D/gendev-macos/releases)
- [RetroArch](http://www.retroarch.com/?page=platforms)

## Usage

Use the template dir as project base

```bash
cp -r template/ my-game/
```

By default `make` will build and run the ROM in an emulator

```bash
# Compile and run a specific project
make -C hello/

# Compile and run current project
cd hello/ && make
```

If you'd like to trigger each step manually:

```bash
cd hello/

# Compile source
make build

# Run binary via RetroArch
make run
```

Built with these awesome tools:

- [SGDK](https://github.com/Stephane-D/SGDK)
- [Gendev](https://github.com/kubilus1/gendev)
- [RetroArch](https://github.com/libretro/RetroArch)
