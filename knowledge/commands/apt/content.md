# apt - manage packages on Debian-based systems

`apt` is an interactive package-management command used on Debian, Ubuntu, and
related distributions. It is not portable to distributions using other
package managers.

```bash
apt search PACKAGE
apt show PACKAGE
sudo apt update
sudo apt install PACKAGE
```

Searching and inspecting package metadata need no special privileges. Updating
package lists and changing installed software require administrator
authorization. `apt update` refreshes metadata; it does not upgrade installed
packages.

Before a broad upgrade, read the proposed changes and ensure important data is
backed up. `apt remove` can also remove dependent packages, and `apt autoremove`
may propose packages that are still useful to you. Review the transaction
summary before confirming. Scripts should generally use the more stable
`apt-get` interface rather than parse `apt` output.
