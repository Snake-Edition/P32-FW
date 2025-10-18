@echo off
for /f "usebackq delims=" %%a in ("version.txt") do (
    set "version=%%a"
    goto :break
)
:break

::Ensure required build tools are present (silent install of VC tools workload)
winget install --id=Microsoft.VisualStudio.2022.BuildTools -e --silent --override "--quiet --wait --norestart --nocache --installPath C:\\BuildTools --add Microsoft.VisualStudio.Workload.VCTools" >nul 2>&1
::Ensure Python 3.11 is available for better Windows wheel support
winget install --id=Python.Python.3.11 -e --silent >nul 2>&1

echo Setting up Python virtual environment...

::Check if .venv exists, if not create it
if not exist ".venv" (
    echo Creating virtual environment...
    ::Prefer Python 3.11 if available (more prebuilt wheels)
    py -3.11 -m venv .venv 2>nul || python -m venv .venv
)

::Activate virtual environment
echo Activating virtual environment...
call .venv\Scripts\activate.bat

::Install/upgrade pip and required packages
echo Installing dependencies...
python -m pip install --upgrade pip
python -m pip install -r requirements.txt

echo on


python utils/build.py --bootloader empty --preset mini-en-cs --final
move /Y "build\mini-en-cs_release_emptyboot\firmware.bbf" "build\Snake_MINI_core_en-cs_%version%.bbf"

pause
