Changes to version 1.95a by RayeR, 13.2.2011
============================================
I added support for following VESA 1.1 videomodes:
1280x1024/15bpp, 1280x1024/16bpp
1600x1200/15bpp, 1600x1200/16bpp
Because the 1600x1200 videomode numbers are not standartized in VESA VBE
and depends on VGA BIOS writers (may differ on nVidia/ATI/Intel/etc...)
it's possible to override default videomode numbers by setting environment
variables. Default values are suited for nVidia cards (tested on 7900GT)
set VM1600HI15=0x11D
set VM1600HI16=0x146
You can use my VESATEST.EXE utility to get VESA mode list.

My testing Real DOS Machine:
============================
CPU: Intel Core 2 Duo E8400 (@3,5GHz)
VGA: nVidia GeForce 7900GT/256M/PCI-E
NET: Realtek 8111/8168 gigabyte ethernet, ODI driver 1.34, ODIPKT 3.1 wrapper
OS:  MS-DOS 6.22, JemmEx 5.73 (2 GB XMS), UIDE.SYS 25.7.2010, CuteMouse 2.1b4

Quick step by step guide how to compile Arachne from sources
============================================================
 1) Prepare your compilers. I used Borland C++ 3.1 and TurboAssembler 4.1 for DOS
   and have set path to BC31\BIN to access needed utils.
   Warning: Older version of TurboAssembler crashes when compiling LOPIF library!
 2) Start BC and load LOPIF\VV.PRJ project in IDE (Project/Open project...)
 3) Check your compiler paths (Options/Directories...)
 4) Compile LOPIF (Compile/Build all), it will show 42 warnings, ignore it
 5) Go to DOS console (File/DOS shell)
 6) Run HIXLOV.BAT, this will build the HIXLOV.LIB and copy it to Arachne sources
 7) Type exit to go back to BC IDE
 8) Close LOPIF project (Project/Close project)
 9) Load CORE-386.PRJ project in IDE (Project/Open project...)
10) Check your compiler paths (Options/Directories...)
11) Compile Arachne (Compile/Build all), it will show 0 warnings
12) Copy your new CORE-386.EXE as CORE.EXE into your Arachne directory
