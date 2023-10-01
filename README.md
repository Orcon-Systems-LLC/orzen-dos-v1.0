# orzn-dos
souce and testing repo


## Building
First, install the following dependencies:

```
# Ubuntu, Debian:
$ sudo apt install build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo \
                   nasm mtools qemu-system-x86
           
# Fedora:
$ sudo dnf install gcc gcc-c++ make bison flex gmp-devel libmpc-devel mpfr-devel texinfo \
                   nasm mtools qemu-system-x86
                   
# Arch & Arch-based:
$ sudo pacman -S gcc make bison flex libgmp-static libmpc mpfr texinfo nasm mtools qemu-system-x86
```
NOTE: to install all the required packages on Arch, you need an [AUR helper](https://wiki.archlinux.org/title/AUR_helpers).

After that, run `make toolchain`, this should download and build the required tools (binutils and GCC). If you encounter errors during this step, you might have to modify `build_scripts/config.mk` and try a different version of **binutils** and **gcc**. Using the same version as the one bundled with your distribution is your best bet.

Finally, you should be able to run `make`. Use `./run.sh` to test your OS using qemu.
