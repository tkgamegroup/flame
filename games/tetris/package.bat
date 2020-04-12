@echo off
set /p config=
"../../bin/debug/package_maker.exe" %config%
pause
