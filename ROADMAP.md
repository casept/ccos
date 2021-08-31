# Roadmap

## Toolchain

* [ ] Build x86_64 without redzone in libgcc
* [ ] Contribute patch to crosstool-ng to make this easier

## Build system

* [ ] Switch from cmake to GNU make

## Bootloader

* [ ] Load kernel into RAM via BIOS
* [X] Enter protected mode
* [X] Run first C code
* [X] VGA debug output
* [ ] ELF parser and loader
* [ ] Stivale2 parser and loader
* [ ] Enter long mode
* [ ] Relocate and boot kernel

## Kernel

* [ ] Get basic text output working (maybe via stivale2 console at first)
* [ ] Write a VGA text mode driver
* [ ] Write a (PS/2) keyboard driver
* [ ] Write a simple debug shell
* [ ] Write a filesystem driver (FAT32 would be easy, but need to clear up whether M$ still has patent on VFAT)
* [ ] Put fundamental microkernel abstractions in place
  * [ ] Memory protection
  * [ ] Processes
  * [ ] Message passing
  * [ ] Move drivers out of the kernel
