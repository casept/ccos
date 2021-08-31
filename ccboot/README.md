# ccboot

`ccboot` (Casept's Crappy Bootloader) is a minimum-effort x86_64 bootloader.

It's boot protocol is a stripped-down version of Stivale 2, basically only implementing features needed by `cccore`.
It doesn't support features such as non-ELF kernel images, the Stivale console, loading 32-bit kernels etc.

Also, for now it can only load fairly small kernels from an (emulated) floppy that must also contain the bootloader.
