rem Install virtualenv
python -m pip install --user virtualenv

rem Create virtualenv
python -m venv .venv

rem EN-CS FW
python utils/build.py --generate-bbf --bootloader empty --preset mini-en-cs --final

rem EN-DE FW
python utils/build.py --generate-bbf --bootloader empty --preset mini-en-de --final

rem EN-ES FW
python utils/build.py --generate-bbf --bootloader empty --preset mini-en-es --final

rem EN-FR FW
python utils/build.py --generate-bbf --bootloader empty --preset mini-en-fr --final

rem EN-IT FW
python utils/build.py --generate-bbf --bootloader empty --preset mini-en-it --final

rem EN-PL FW
python utils/build.py --generate-bbf --bootloader empty --preset mini-en-pl --final

rem coreXY FW
python utils/build.py --generate-bbf --bootloader empty --preset minixy --final

rem long bed (250 mm) FW
python utils/build.py --generate-bbf --bootloader empty --preset miniLG --final

rem python utils/build.py -h
pause
