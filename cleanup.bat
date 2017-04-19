cd %~dp0

REM msbuild.exe cali.sln /t:Clean /property:Configuration=Debug

del cali.VC.db /F
rmdir /s /q ".vs"
rmdir /s /q "x64"
rmdir /s /q "ipch"