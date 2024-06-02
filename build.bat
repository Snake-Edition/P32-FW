rem Original FW
python utils/build.py --generate-bbf --bootloader empty --preset mini --final

rem coreXY FW
python utils/build.py --generate-bbf --bootloader empty --preset minixy --final

rem long bed (250 mm) FW
python utils/build.py --generate-bbf --bootloader empty --preset miniLG --final

rem i3 MK3.3 upgrade FW
python utils/build.py --generate-bbf --bootloader empty --preset mini_i3_mk33 --final

rem python utils/build.py -h
pause
