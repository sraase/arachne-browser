@echo off
if not exist c:\pedit\pedit.exe goto err
c:\pedit\pedit.exe %1
rem change drive:\path\filename
rem in both lines above to point to your editor of choice
goto done
:err
echo No external editor setup
pause
:done