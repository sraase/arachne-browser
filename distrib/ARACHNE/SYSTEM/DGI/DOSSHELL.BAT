@echo off
set oldpmt=%prompt%
set prompt=Type EXIT to return to Arachne$_$p$g
if exist ppp.log set prompt=You are still online$_Type EXIT to return to Arachne$_$p$g
if (%COMSPEC%)==() goto nocomspec
%COMSPEC% /E:1024
goto end
:nocomspec
COMMAND.COM /E:1024
:end
set prompt=%oldpmt%
set oldpmt=

rem correct incompatibility with FreeDos (and perhaps others),
rem in-which simply 'drive:' is requred to change to the correct drive
rem rather than 'drive:\path\' which generates an error
rem %1 will now be only 'drive:' via $z in dosshell.dgi in mime.cfg
%1

rem old line
rem cd %1system

rem new line
cd %2system
rem %2 will now be the same as the old %1 ... 'drive:\exepath\' via $e
cd ..
