This is Doom. ported from prboom

Installation
============

This should be compiled with Android 2.2 SDK and NDK r4b - google for them and install them as described in their docs
(the application will run on Android 1.6 and above).
You'll need to install Ant too.
The most supported environnment for that port is Linux, MacOs should be okay too, 
If you're developing on Windows you'd better install andLinux or Ubuntu+Wubi, to get proper Linux environment
running inside Windows, then install Linux toolchain on it. I was told andLinux compiles faster than Cygwin.
Also you'll need full set of Linux utils and symlinks support to launch ChangeAppSettings.sh (sh, grep, sed, tr).

How to compile Doom
=============================================

1.Go to "project" directory and launch command
	android update project -p .

2.run ./debug.sh to generate a debug version doom or run ./release.sh to generate a release version doom