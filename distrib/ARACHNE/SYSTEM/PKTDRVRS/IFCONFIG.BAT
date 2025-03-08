@echo off
rem Please consider this batch file to be rather joke for Linux users ;-)
rem Please change packet driver name and parameters on following line:
if (%1)==(up) ne2000 0x60 10 0x300
if (%1)==(down) ..\..\termin 0x60
echo.
pktchk 0x60
if errorlevel 1 goto no
pktstat 0x60
goto end
:no
echo No packet driver installed - ethernet interface is down.
:end
echo.
echo DOS ifconfig for packet driver ethernet interface, (G)2000 Arachne Labs
echo.
echo Usage: ifconfig, ifconfig up, ifconfig down
