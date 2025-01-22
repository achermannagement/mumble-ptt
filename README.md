Building
--------

Fedora
======

This is using Fedora 41 as the build system.
You need to install these packages to build

- libinput-devel
- libgudev-devel
- dbus-devel


Ubuntu
======

This has just been tested with Ubuntu 24.10 (Oracular Oriole)
You need to install this package to build

- libinput-dev

[!IMPORTANT]
The snap of mumble does not work with this. If you installed mumble through the
App Center of Ubuntu you likely have the snap installed! Uninstall the snap through
the App Center and then install the deb package in the terminal like so:

`sudo apt-get install mumble`

Compile
=======

`gcc -Wall -Werror -Wextra -o mumble_ptt mumble_ptt.c -linput -ludev`

It needs to be run under the input permission group to work
DO NOT run as superuser as that will change the environment and break the dbus-send commands!

```
usermod -aG input <user>
newgrp input
```
