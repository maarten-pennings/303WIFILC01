@ECHO off
IF "(env) " neq "%PROMPT:~0,6%" ECHO Please run setup.bat first && EXIT /b

ECHO You might have to update the --port argument
ECHO.
python  env\Lib\site-packages\esptool.py  --port COM12  --baud 115200  read_flash 0x0000 1048576  backup.bin
ECHO.
ECHO Done

REM python  env\Lib\site-packages\esptool.py  --port COM9  --baud 115200  erase_flash  write_flash 0x0000 backup.bin

REM python  env\Lib\site-packages\esptool.py  --port COM9  --baud 115200  erase_flash
REM python  env\Lib\site-packages\esptool.py  --port COM9  --baud 115200  erase_flash  write_flash 0x0000 ..\5-clock\nCLC.ino.bin
