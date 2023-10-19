rem Original FW
python utils/build.py --generate-bbf --bootloader empty --preset mini --final

rem coreXY FW
python utils/build.py --generate-bbf --bootloader empty --preset minixy --final

rem long bed (250 mm) FW
python utils/build.py --generate-bbf --bootloader empty --preset miniLG --final

rem python utils/build.py -h
pause
