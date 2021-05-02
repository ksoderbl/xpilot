This is the README for XPilot.exe and XPilotServer.exe  4.5.5-NT13.

Copyright � 1991-2002 by Bj�rn Stabell, Ken Ronny Schouten, Bert Gijsbers & Dick Balaska.

Following are notes for building the
Windows version of XPilot.  See INSTALL in the XPilot root
directory for the UNIX instructions.

This file (README.txt) is included in the source distribution
for XPilot.  You do not need to build XPilot for Windows.
You only need to install the binary distribution available
from www.xpilot.org

There is 1 Project Workspace file: XPilot.dsw
which loads the two makefiles:
     src/client/NT/xpilot.dsp     - The client
     src/server/NT/xpilots.dsp   - The server

The development executables run from the xpilot root directory, so as to gain 
access to the lib subdirectory.

XPilot.ini is distributed in the src/client/NT directory.  You should copy 
this file to the xpilot root directory (or a default one without comments 
will be created)

These days, only VC++ 6.0 is officially supported.  Although there is nothing to
prevent it from being built on older versions, noone can maintain the makefiles.
