# Roadmap

## Toolchain

* [ ] Build x86_64 without redzone in libgcc
* [ ] Contribute patch to crosstool-ng to make this easier

## Build system

* [X] Switch from cmake to GNU make
* [ ] Make kernel image build without root
* [ ] Allow setting per-target debug/release

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

* [X] Get basic text output working (maybe via stivale2 console at first)
* [x] Write a VGA text mode driver
* [ ] Write a (PS/2) keyboard driver
* [ ] Write a simple debug shell
* [ ] Write an ext2 filesystem
* [ ] Put fundamental microkernel abstractions in place
  * [ ] Interrupts
  * [ ] System timer
  * [ ] Memory protection
  * [ ] Processes
    * [ ] Scheduling
    * [ ] ELF loading
  * [ ] Message passing
  * [ ] Move drivers out of the kernel
