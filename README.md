Building
--------

This is using Fedora 41 as the build system.
You need to install these packages to build

- libinput-devel
- libgudev-devel
- dbus-devel

then run

`gcc -Wall -Werror -Wextra -o mumble_ptt mumble_ptt.c -linput -ludev -ldbus-1`

or if you are unfortunate

`gcc -Wall -Werror -Wextra $(pkg-config dbus-1 --cflags) -o mumble_ptt mumble_ptt.c -linput -ludev -ldbus-1`

It needs to be run under the input permission group to work
DO NOT run as superuser as that will change the environment and break the dbus-send commands!

```
usermod -aG input <user>
newgrp input
```
