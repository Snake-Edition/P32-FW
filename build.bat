echo off
for /f "usebackq delims=" %%a in ("version.txt") do (
    set "version=%%a"
    goto :break
)
:break
echo on


rem coreXY
python utils/build.py --bootloader empty --preset minixy-en-cs --final
move /Y "build\minixy-en-cs_release_emptyboot\firmware.bbf" "build\Snake_MINI_coreXY_en-cs_%version%.bbf"

pause
