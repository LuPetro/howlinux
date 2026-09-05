# pacman - manage packages on Arch Linux

`pacman` manages software from configured repositories on Arch Linux.
Replace `PACKAGE` with the package name or search term you need.

Search repositories, inspect a package, or list installed packages without
administrator privileges:

```bash
pacman -Ss PACKAGE
pacman -Si PACKAGE
pacman -Q
```

Before upgrading, check Arch Linux news for manual intervention notices and
back up important data. Review the proposed transaction before confirming.
These commands require administrator authorization and upgrade the whole
system; the second also installs the requested package:

```bash
sudo pacman -Syu
sudo pacman -Syu PACKAGE
```

Arch is a rolling release distribution and does not support partial upgrades.
Do not refresh repository databases with `pacman -Sy` and then install a
package without completing a full system upgrade. If an upgrade fails,
resolve it before installing additional packages.

Removing a package changes installed software. Review the transaction and
dependency checks before confirming:

```bash
sudo pacman -R PACKAGE
```

The Arch User Repository (AUR) contains user-submitted build recipes and is
separate from the configured binary repositories; `pacman -S` does not build
AUR packages.

References: [pacman manual](https://man.archlinux.org/man/pacman.8.en),
[system maintenance](https://wiki.archlinux.org/title/System_maintenance),
[Arch Linux news](https://archlinux.org/news/).
