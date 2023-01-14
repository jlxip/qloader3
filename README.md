# qloader3

qloader3 is a highly opinionated stripped down fork of the `v3.0-branch` of [the Limine bootloader](https://github.com/limine-bootloader/limine). It removes all boot protocols but stivale2, as well as all filesystems except FAT32 and ISO9660. Support for IA-32, PXE, and background images is also dropped.

## History
In the beginning (circa 2020), there was [the Stivale boot protocol](https://github.com/stivale/stivale/blob/master/STIVALE.md), a pretty neat, albeit limited, boot protocol for hobby operating system projects. Given its weaknesses, [Stivale2](https://github.com/stivale/stivale/blob/master/STIVALE2.md) was created, which is simple (yet powerful) enough to write a bare bones kernel pretty quickly. It's beautiful: easy to understand, easy to implement, and easy to extend once your kernel gets bigger.

Stivale was the native boot protocol of a bootloader called qloader2, later renamed to _Limine_. Since then, Limine kept developing, adding new features and supporting more filesystems and boot protocols (and even UEFI). Then, the project developed _the Limine boot protocol_, and, in version 4.0, the support for stivale (and stivale2) was removed.

The branch for Limine v3.0 was kept updated until January 1st, 2023. This is a fork of that state, moved to `master`. qloader3 comes up just because Limine is, at the time I write this, the space-wise bottleneck of my OS, [Daisōgen](https://github.com/daisogen), and I want a smaller ISO.

## Compile
To compile qloader3, just run:
```bash
./bootstrap
./configure --enable-all
make
```

You'll then find everything under the `bin` directory.
