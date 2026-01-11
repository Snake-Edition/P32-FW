echo off
for /f "usebackq delims=" %%a in ("version.txt") do (
    set "version=%%a"
    goto :break
)
:break
echo on

rem MINI
python utils/build.py --bootloader empty --preset mini-en-cs --final
move /Y build\mini-en-cs_release_emptyboot\firmware.bbf "build\Snake_MINI_en-cs_%version%.bbf"
python utils/build.py --bootloader empty --preset mini-en-de --final
move /Y build\mini-en-de_release_emptyboot\firmware.bbf "build\Snake_MINI_en-de_%version%.bbf"
python utils/build.py --bootloader empty --preset mini-en-es --final
move /Y build\mini-en-es_release_emptyboot\firmware.bbf "build\Snake_MINI_en-es_%version%.bbf"
python utils/build.py --bootloader empty --preset mini-en-fr --final
move /Y build\mini-en-fr_release_emptyboot\firmware.bbf "build\Snake_MINI_en-fr_%version%.bbf"
python utils/build.py --bootloader empty --preset mini-en-it --final
move /Y build\mini-en-it_release_emptyboot\firmware.bbf "build\Snake_MINI_en-it_%version%.bbf"
python utils/build.py --bootloader empty --preset mini-en-pl --final
move /Y build\mini-en-pl_release_emptyboot\firmware.bbf "build\Snake_MINI_en-pl_%version%.bbf"
python utils/build.py --bootloader empty --preset mini-en-jp --final
move /Y build\mini-en-jp_release_emptyboot\firmware.bbf "build\Snake_MINI_en-jp_%version%.bbf"
python utils/build.py --bootloader empty --preset mini-en-uk --final
move /Y build\mini-en-jp_release_emptyboot\firmware.bbf "build\Snake_MINI_en-uk_%version%.bbf"

rem coreXY
python utils/build.py --bootloader empty --preset minixy-en-cs --final
move /Y "build\minixy-en-cs_release_emptyboot\firmware.bbf" "build\Snake_MINI_coreXY_en-cs_%version%.bbf"
python utils/build.py --bootloader empty --preset minixy-en-de --final
move /Y "build\minixy-en-de_release_emptyboot\firmware.bbf" "build\Snake_MINI_coreXY_en-de_%version%.bbf"
python utils/build.py --bootloader empty --preset minixy-en-es --final
move /Y "build\minixy-en-es_release_emptyboot\firmware.bbf" "build\Snake_MINI_coreXY_en-es_%version%.bbf"
python utils/build.py --bootloader empty --preset minixy-en-fr --final
move /Y "build\minixy-en-fr_release_emptyboot\firmware.bbf" "build\Snake_MINI_coreXY_en-fr_%version%.bbf"
python utils/build.py --bootloader empty --preset minixy-en-it --final
move /Y "build\minixy-en-it_release_emptyboot\firmware.bbf" "build\Snake_MINI_coreXY_en-it_%version%.bbf"
python utils/build.py --bootloader empty --preset minixy-en-pl --final
move /Y "build\minixy-en-pl_release_emptyboot\firmware.bbf" "build\Snake_MINI_coreXY_en-pl_%version%.bbf"
python utils/build.py --bootloader empty --preset minixy-en-jp --final
move /Y "build\minixy-en-jp_release_emptyboot\firmware.bbf" "build\Snake_MINI_coreXY_en-jp_%version%.bbf"
python utils/build.py --bootloader empty --preset minixy-en-uk --final
move /Y "build\minixy-en-jp_release_emptyboot\firmware.bbf" "build\Snake_MINI_coreXY_en-uk_%version%.bbf"

rem i3 MK3.3
python utils/build.py --bootloader empty --preset mini_i3_mk33-en-cs --final
move /Y build\mini_i3_mk33-en-cs_release_emptyboot\firmware.bbf "build\Snake_MINI_i3_MK33_en-cs_%version%.bbf"
python utils/build.py --bootloader empty --preset mini_i3_mk33-en-de --final
move /Y build\mini_i3_mk33-en-de_release_emptyboot\firmware.bbf "build\Snake_MINI_i3_MK33_en-de_%version%.bbf"
python utils/build.py --bootloader empty --preset mini_i3_mk33-en-es --final
move /Y build\mini_i3_mk33-en-es_release_emptyboot\firmware.bbf "build\Snake_MINI_i3_MK33_en-es_%version%.bbf"
python utils/build.py --bootloader empty --preset mini_i3_mk33-en-fr --final
move /Y build\mini_i3_mk33-en-fr_release_emptyboot\firmware.bbf "build\Snake_MINI_i3_MK33_en-fr_%version%.bbf"
python utils/build.py --bootloader empty --preset mini_i3_mk33-en-it --final
move /Y build\mini_i3_mk33-en-it_release_emptyboot\firmware.bbf "build\Snake_MINI_i3_MK33_en-it_%version%.bbf"
python utils/build.py --bootloader empty --preset mini_i3_mk33-en-pl --final
move /Y build\mini_i3_mk33-en-pl_release_emptyboot\firmware.bbf "build\Snake_MINI_i3_MK33_en-pl_%version%.bbf"
python utils/build.py --bootloader empty --preset mini_i3_mk33-en-jp --final
move /Y build\mini_i3_mk33-en-jp_release_emptyboot\firmware.bbf "build\Snake_MINI_i3_MK33_en-jp_%version%.bbf"
python utils/build.py --bootloader empty --preset mini_i3_mk33-en-uk --final
move /Y build\mini_i3_mk33-en-jp_release_emptyboot\firmware.bbf "build\Snake_MINI_i3_MK33_en-uk_%version%.bbf"

rem i3 MK3.5 coreXY
python utils/build.py --bootloader empty --preset i3xy_mk3.5 --final
move /Y "build\i3xy_mk3.5_release_emptyboot\firmware.bbf" "build\Snake_i3_MK3.5_coreXY_%version%.bbf"

pause
