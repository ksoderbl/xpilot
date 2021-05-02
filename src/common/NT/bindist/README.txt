$Id: README.txt.msub,v 1.1 1998/04/17 04:19:48 dick Exp $

This is the README for XPilot.exe and XPilots.exe  4.1.0-NT13.

Copyright © 1991-1998 by Bjørn Stabell, Ken Ronny Schouten, Bert Gijsbers & Dick Balaska.

Following are notes for building the
Windows version of XPilot.  See INSTALL in the XPilot root
directory for the UNIX instructions.

This file (README.txt) is included in the source distribution
for XPilot.  You do not need to build XPilot for Windows.
You only need to install the binary distribution available
from www.xpilot.org

There are 2 makefiles:
     contrib/NT/xpilot/xpilot.mak     - The client
     contrib/NT/xpilots/xpilots.mak   - The server

The development executables run from the xpilot root directory, so as to gain access
to the lib subdirectory.

XPilot.ini is distributed in the contrib/NT/xpilot directory.  You should copy this
file to the xpilot root directory (or a default one will be created)

XPilot.exe and XPilots.exe have been successfully built
using Visual C++ 4.1, 4.2, and 5.0


***DOS FILE ALERT!
Visual C++ deals with *.c and *.h files in a UNIX format nicely.
However, the machine readable files (*.mak, *.rc, *.clw) need to be
in a DOS (CRLF) format.  For ease of maintenence, all files in the contrib/NT
subtree will be in the DOS file format.  All other files are in UNIX format.
(These will look like crap in Notepad.)
