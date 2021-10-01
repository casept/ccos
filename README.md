# CCOS

CCOS (Casept's Crappy Operating System) is my taking a crack at writing an x86_64 OS, largely from scratch from the bootloader up.

It will probably end up being a fairly classical microkernel design. The main goal here is learning, not revolutionizing OS design.

## Building

The build is driven by some more-or-less gross looking GNU Make scripts, utilizing fully statically linked toolchain binaries which are checked into the repo for convenience. Building on a non-Linux OS is unsupported.

Tools other than the toolchain can be pulled in via Nix:

```shell
nix-shell
make -j$(nproc)
```
