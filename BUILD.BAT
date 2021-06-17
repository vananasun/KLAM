@echo off
if exist BUILD\KLAM.EXE del BUILD\KLAM.EXE
mingw32-make
if exist BUILD\KLAM.EXE dosbox -c "runwcom.bat" -noconsole
exit