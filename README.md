openBCM <= 2.x (1.xx)
======================

OpenBCM - BCM for everybody (packet radio mailbox system initiated by DL8MBT)

The initial commit is equal to the sources available from the maintainer DH8YMB,
http://dnx274.org/baybox/pre/sobcm-1.07b12.tgz

build on 64 bit linux systems:

aptitude install libc6-dev-i386 lib32stdc++-8-dev
make

This repository stays the "old-school" port of the openBCM.
Here we have focus on bugfixes while supporting all operating systems as always (DOS, WIN32, LINUX).

Also feature implementation is done but in a very conservative way.
For more ambitious development approach have a look the the >= 2.x port.

openBCM >= 2.x
==============
There exists also a fork of this repository at

https://github.com/andreaspeters/openBCM

Andreas, DC6AP maintaines this port.
The main concern is implementing new features with having focus on linux based systems.

---

Maintainers of both ports stay in contact and report each other if some relevant stuff does pop up.
