@echo off
REM run.cmd - wrapper for run.ps1 (double-click friendly)
REM Usage: run.cmd [dev|release] [-Clean] [-NoBuild] [-NoRun]
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0run.ps1" %*
