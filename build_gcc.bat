@Echo off
Pushd %~dp0
SetLocal EnableDelayedExpansion
Set Args=--std=c++0x -Wno-attributes -O3 -msse2 -mfpmath=sse -flto -s -o "Leanify" Leanify.res 
For /r "%~dp0" %%i In (*.cpp *.c *.cc) Do (Set t=%%i && Set Args=!Args!!t:%~dp0=!)
For %%i In (formats\mozjpeg\jdcolext.c formats\mozjpeg\jdmrgext.c) Do Set Args=!Args:%%i =!
windres --output-format=coff Leanify.rc Leanify.res
g++ %Args%
Del Leanify.res
Pause