@echo off
break=off>NUL
rem --- Arachne is *.BAT which will make your PC survive to 21st century! ---
rem ------ (c)1996-2002 Michael Polak, Arachne Labs, http://arachne.cz/ -----
if (%1) == (-u) goto termin
if not (%1) == () goto continue
if not exist arachne.cfg setup.bat
if not exist arachne.pck setup.bat
if exist lock arachne.bat system\gui\err_lock.ah
if exist ppp.log arachne.bat -r
echo Don't worry, this is just Arachne lock file...>lock
:continue
if not "%TRACE%"=="on" goto skip1
echo >>trace.log #1 %1
echo >>trace.log #2 %2
:skip1
core.exe %1 %2
if errorlevel 128 goto fastexecute
if errorlevel 64 goto execute
if errorlevel 32 goto end
if errorlevel 16 goto graphics
if errorlevel 8 goto resolution
REM *
if errorlevel 4 echo Errorlevel 4
if errorlevel 2 echo Errorlevel 2
if errorlevel 1 echo Errorlevel 1
if errorlevel 0 echo Errorlevel 0
REM *
pause
core.exe system\gui\fatal.ah
if errorlevel 128 goto fastexecute
if errorlevel 64 goto execute
if errorlevel 32 goto end
if errorlevel 16 goto graphics
if errorlevel 8 goto resolution
if not exist core.exe goto noexe
goto err
:termin
termin.com 0x60
del ppp.log
goto end
:noexe
echo CORE.EXE is missing.
goto end
:graphics
if not "%TRACE%"=="on" goto skip2
echo >>trace.log graphics fail
:skip2
setup.bat
:resolution
core -s
if errorlevel 32 goto end
goto continue
:execute
$roura$.bat
goto err
:fastexecute
if not (%ARACHNETEMP%)==() %ARACHNETEMP%\$roura$.bat
if exist %TEMP%\ARACHNE.TMP\$roura$.bat %TEMP%\ARACHNE.TMP\$roura$.bat
if exist %TEMP%\$roura$.bat %TEMP%\$roura$.bat
REM Compatibility with old %TEMP% format maintained
:err
echo Abnormal Arachne termination. Please reboot system and/or restart Arachne.
:end
if exist $roura?.bat del $roura?.bat
if exist %ARACHNETEMP%\$roura$.bat del %ARACHNETEMP%\$roura$.bat
if exist %TEMP%\$roura$.bat del %TEMP%\$roura$.bat
if exist *._$b del *._$b
if exist %ARACHNETEMP%\*._$b del %ARACHNETEMP%\*._$b
if exist %TEMP%\*._$b del %TEMP%\*._$b
if exist ip-up.bat del ip-up.bat
if exist ppp.log echo Type 'arachne -u' to terminate SLIP/PPP connection.
if exist lock del lock
if not exist %TEMP%\ARACHNE.TMP\*.* goto done
for %%f in (%TEMP%\arachne.tmp\*.*) do del %%f
rd %TEMP%\arachne.tmp>NUL
:done
echo.