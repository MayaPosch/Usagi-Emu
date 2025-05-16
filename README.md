# Usagi Electric Rabbit Hole 

This repository contains projects related to Usagi Electric's DIY computer systems, in particular emulators.

For now there is just one active folder, for the UE1 valve-based computer system. 

## UE1

UE1 is a simple 1-bit computer system. The emulators in this folder are written both in C++ (WIP) and Ada (WIP). Simply use `make` to build them.

Usage instructions can be obtained by running with the `-h` flag, but essentially you use `-f <path>` to provide an ASM file, and `-c` to set a specific clock speed (default 10 Hz).

The only dependency ([Sarge](https://github.com/MayaPosch/Sarge)) is already included in the source.

[!NOTE]
The original QuickBasic version is also contained in this repository. Its copyright is owned by David Lovett (Mr. Usagi Electric) himself, not by this author.
