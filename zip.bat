if exist asrc.zip del asrc.zip
pkzip -P asrc.zip *.c js\*.c icq\*.c *.h js\*.h icq\*.h *.asm *.hsm *.lib *.prj *.pro zip.bat
