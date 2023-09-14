
Source code of LOPIF library for DOS
------------------------------------

This is source code of LOPIF graphics library for DOS, originally
(C) 1990-1999 Zdenek Harovnik, Ibase Group. It was subsequently
maintained by Arachne Labs, http://www.arachne.cz/, then released
under the terms of the GNU General Public License, as published
by the Free Software Foundation (version 2 of the License or, at
your option, any later version).

It is currently maintained, to a limited extent, by the Arachne
Development group at : groups.yahoo.com/group/ArachneDevelopment/

No sample files are available so far, sorry. However, compatibility layer
called LINGLUE exists for porting graphical applications between 16bit
DOS (using this library) and 32bit Linux (SVGAlib), and maybe other
platforms similar to POSIX. This multiplatform enviroment is currently
undocumented and not fully tested. The intent of LINGLUE is to form an
"ideal" platform for writing/porting smaller apps (eg. games) between
16bit DOS, where all modern video cards provide 16bit VESA BIOS interface,
and 32bit Linux world, using SVGAlib, and in future maybe Gtk, Qt
or directly Xlib libraries. In future, Arachne WWW browser may provide
natural HTML widget as cornerstone of more advanced GUI toolkit. LINGLUE
source code, as it exists, is included with the Arachne sources. Further
information on LINGLUE might be available from the Arachne Development
group or from Arachne Labs (xchaos@arachne.cz).

LINGLUE = DOS/LOPIF <-> Linux/SVGAlib portability

How to use LOPIF
----------------

1. #include "x_lopif.h"
2. add HIXLOV.LIB to your Borland C project file.

Very simple example can be found in vv.cc file, although comments are
unforunately in Czech. For more information, contact xchaos@arachne.cz
No english documentation is available so far, sorry.

How to build LOPIF
------------------

LOPIF with virtual screens and with hicolor support: HIXLOV.LIB
Note, this is the normal version of the library used by Arachne.

1. Build VV.PRJ -> VV.EXE
2. Run HIXLOV.BAT

LOPIF without virtual screens and with hicolor support: HIXLO.LIB

1. Build VV.PRJ -> VV.EXE
2. Run HIXLO.BAT

LOPIF without virtual screens and without hicolor support: ARAXLO.LIB

1. Build ARAXLO.PRO -> ARAXLO.EXE
2. Run ARAXLO.BAT

LOPIF with virtual screens but without hicolor support: ARAXLOV.LIB

1. Build ARAXLO.PRO -> ARAXLO.EXE
2. Run ARAXLOV.BAT

