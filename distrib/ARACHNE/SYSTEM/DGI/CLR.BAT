@echo off
rem uncomment the following 4 lines to be warned before clearing
rem echo ******************* WARNING *************
rem echo You are about to delete all files in the Cache
rem choice Continue & Delete
rem if errorlevel 2 goto end
rem :Continue&Delete
REM usage: clrcache.dgi (cache.idx) (cache path) (TEMP path) (filemask)
REM cache path or TEMP path MUST NOT be the same as Arachne installation dir.!
if "%3"=="" goto end
REM The above catches any call to CLR.bat without at least 3 parameters !
if exist %1 del %1
if exist %3$roura$.BAT copy >nul %3$roura$.BAT bat.tmp
for %%f in (%2*.*) do del %%f
for %%f in (%2headers\*.*) do del %%f
for %%f in (%3*.*) do del %%f
if exist bat.tmp copy >nul bat.tmp %3$roura$.BAT
if exist bat.tmp del bat.tmp
for %%x in (Yes yes YES) do if "%4"=="%%x" del %5
:end
