@echo off
set ASETUP=
set ASWITCH=
if (%1)==(os) goto setup_os
if (%1)==(inst) set ASETUP=inst
if exist arachne.cfg goto setup_gr
if not exist hotlist.htm copy hot_mgr.htp hotlist.htm>NUL
copy 486pc.acf arachne.cfg
:setup
core -s
if errorlevel 32 goto end
if errorlevel 2 goto abort
call arachne.bat reload:file:setup_os.dgi
goto end
:setup_os
call arachne.bat reload:file:setup_os.dgi
goto end
:setup_gr
core -s
if errorlevel 32 goto end
if errorlevel 2 goto abort
call arachne.bat setup.htm
:end
echo Setup finished. Type "arachne" to launch configured Arachne WWW browser.
echo.
:abort
set ASETUP=
set ASWITCH=