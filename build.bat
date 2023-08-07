echo off
cls

tcc main.c -o atomo.exe -Wall -Wl,-subsystem=windows -IC:\tcc\iup\include -LC:\tcc\iup -liup -luser32

if %errorlevel% == 0 (
	atomo.exe
) else (
	echo erro.
)
