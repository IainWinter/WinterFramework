.\tools\premake5.exe vs2019

if not exist .\_bin (
	mkdir .\_bin
)

if not exist .\_bin\SDL.dll (
	
	copy .\vendor\SDL\bin\SDL2.dll .\_bin\
)