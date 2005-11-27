del *.obj
del *.sym
del *.map
markexe.exe core.exe 200000
xcopy /f core.exe c:\arachne\core.exe
rem xcopy core.exe e:\xchaos\arachne\debug\
