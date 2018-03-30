# Sega Genesis/Megadrive tinkering

> WIP

## Installation

You will need the latest Gendev to compile ROMs

- [Linux](https://github.com/kubilus1/gendev/releases)
- [MacOS](https://github.com/SONIC3D/gendev-macos/releases)

## Usage

By default make will build and run the rom in an emulator

```bash
# Make specified dir
make -C hello/

# Make current dir
cd hello/ && make
```

If you'd like to trigger each step manually:

```bash
# Compile source
make build -C hello/

# Run binary via retroarch
make run
```

Built with the these awesome tools:

- [SGDK](https://github.com/Stephane-D/SGDK)
- [Gendev](https://github.com/kubilus1/gendev)
- [Retroarch](https://github.com/libretro/RetroArch)
