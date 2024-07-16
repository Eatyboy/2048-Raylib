@echo off

mkdir Export
pushd Export
copy ..\build\main.exe .
mkdir res
copy ..\res\* res
popd
