@ECHO off
IF "(env) " neq "%PROMPT:~0,6%" ECHO Please run setup.bat first && EXIT /b

ECHO You might have to update the --port argument
ECHO.
python  env\Lib\site-packages\esptool.py  --port COM12  --baud 115200  read_flash 0x000 1048576  backup.bin
ECHO.
ECHO Done
