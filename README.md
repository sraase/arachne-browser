#  Instructions for DOS version

Start by downloading and installing the "Arachne v1.99;GPL" DOS package from https://www.glennmcc.org/arachne/, the file is a199gpl.zip. Installation is straight-forward, just follow the instructions.

To compile the DOS version, find yourself a copy of Borland C++ 3.1 and install it. Cross compilation is not supported, but DOSBox can be used.

Open the CORE.PRJ (386+), CORE-086.PRJ (PC/XT) or CORE-!87.PRJ (no FPU) project file and select "Compile > Build all". Replace C:\ARACHNE\CORE.EXE with your new version after compilation is done.

The DOS version is stable.

# Instructions for Linux/BSD version

To compile the SDL2 version, simply go to the "sdl2" folder and run "make". The SVGALIB version (in the "svgalib" folder) should also compile - please file a bug if it doesn't. If possible, create a 32-bit binary (add "-m32" to CFLAGS or use "CC=i686-linux-gnu-gcc" in sdl2/makefile). All Linux builds are quite unstable, but 64-bit builds are particularly bad.

I have not created a package for the support files (fonts, icons, user interface, help and configuration files) yet, so you need to install them manually. Start by downloading the "Arachne GPL v1.95;GPL" Linux package from https://www.glennmcc.org/aralinux/, the file is arachne-svgalib-1.95.tgz (link is at the top of the page). Glenn has maintained Arachne for the last 20-odd years. Thank him.

Do not run the install script from the archive. Do not run the arachne-install script either.

From the arachne archive:
```
    copy man/man1/arachne.1 to /usr/share/man/man1/
    copy doc/arachne/html/* to /usr/share/doc/arachne/html/
    copy share/arachne/gui/* to /usr/share/arachne/gui/
    copy share/arachne/ikons/* to /usr/share/arachne/ikons/
    copy share/arachne/iso-8859-1/* to /usr/share/arachne/iso-8859-1/ (include subfolders)
    copy share/arachne/iso-8859-2/* to /usr/share/arachne/iso-8859-2/ (include subfolders)
    copy share/arachne/mime.conf to /usr/share/arachne/mime.cfg (different file name)
    copy share/arachne/templates/toolbar.cfg to /usr/share/arachne/toolbar.cfg
    copy share/arachne/templates/entity.cfg to /usr/share/arachne/entity.cfg
```

Edit your new /usr/share/arachne/mime.cfg:

    in [MIME], replace the image/jpeg, image/png and image/x-png MIME types:

```
image/jpeg               JPG>BMP|convert $1 bmp3:$2
image/png                PNG>BMP|convert $1 bmp3:$2
image/x-png              PNG>BMP|convert $1 bmp3:$2
```
    in [ARACHNE], replace the .jpg, .jpe and .png file types:
```
file/.jpg            >BMP|convert $1 bmp3:$2
file/.jpe            >BMP|convert $1 bmp3:$2
file/.png            >BMP|convert $1 bmp3:$2
```

Create a simple configuration file in $HOME/.arachne/arachne.cfg:
```
# Arachne configuration
HomePage http://www.frogfind.com
Multitasking No
HTTPKeepAlive No
```

