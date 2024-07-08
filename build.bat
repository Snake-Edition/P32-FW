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

rem EN-JP FW (Experimental)
python utils/build.py --generate-bbf --bootloader empty --preset mini-en-jp --final

rem coreXY EN-CS
python utils/build.py --generate-bbf --bootloader empty --preset minixy-en-cs --final

rem coreXY EN-DE
python utils/build.py --generate-bbf --bootloader empty --preset minixy-en-de --final

rem coreXY EN-ES
python utils/build.py --generate-bbf --bootloader empty --preset minixy-en-es --final

rem coreXY EN-FR
python utils/build.py --generate-bbf --bootloader empty --preset minixy-en-fr --final

rem coreXY EN-FR
python utils/build.py --generate-bbf --bootloader empty --preset minixy-en-it --final

rem coreXY EN-PL
python utils/build.py --generate-bbf --bootloader empty --preset minixy-en-pl --final

rem coreXY EN-JP (Experimental)
python utils/build.py --generate-bbf --bootloader empty --preset minixy-en-jp --final

rem long bed EN-CS (250 mm) FW
python utils/build.py --generate-bbf --bootloader empty --preset miniLG-en-cs --final

rem long bed EN-DE (250 mm) FW
python utils/build.py --generate-bbf --bootloader empty --preset miniLG-en-de --final

rem long bed EN-ES (250 mm) FW
python utils/build.py --generate-bbf --bootloader empty --preset miniLG-en-es --final

rem long bed EN-FR (250 mm) FW
python utils/build.py --generate-bbf --bootloader empty --preset miniLG-en-fr --final

rem long bed EN-PL (250 mm) FW
python utils/build.py --generate-bbf --bootloader empty --preset miniLG-en-pl --final

rem long bed EN-JP (250 mm) FW (Experimental)
python utils/build.py --generate-bbf --bootloader empty --preset miniLG-en-pl --final

rem i3 MK3.3 EN-CS
python utils/build.py --generate-bbf --bootloader empty --mini_i3_mk33_en-cs --final

rem i3 MK3.3 EN-DE
python utils/build.py --generate-bbf --bootloader empty --mini_i3_mk33_en-de --final

rem i3 MK3.3 EN-ES
python utils/build.py --generate-bbf --bootloader empty --mini_i3_mk33_en-es --final

rem i3 MK3.3 EN-FR
python utils/build.py --generate-bbf --bootloader empty --mini_i3_mk33_en-fr --final

rem i3 MK3.3 EN-IT
python utils/build.py --generate-bbf --bootloader empty --mini_i3_mk33_en-it --final

rem i3 MK3.3 EN-PL
python utils/build.py --generate-bbf --bootloader empty --mini_i3_mk33_en-pl --final

rem i3 MK3.3 EN-JP (Experimental)
python utils/build.py --generate-bbf --bootloader empty --mini_i3_mk33_en-jp --final


rem python utils/build.py -h
pause
