CORE=$(HOME)/.config/retroarch/cores/genesis_plus_gx_libretro.so
ROM=out/rom.bin

default: build run

build:
	make -f $(GENDEV)/sgdk/mkfiles/makefile.gen clean all

run:
	retroarch -L $(CORE) $(ROM)
