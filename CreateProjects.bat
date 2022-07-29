.\tools\premake5.exe vs2022

if not exist .\_bin (
	mkdir .\_bin
)

if not exist .\_bin\SDL.dll (
	copy .\vendor\SDL\bin\SDL2.dll .\_bin\
	copy .\vendor\fmod\bin\fmod.dll .\_bin\
	copy .\vendor\fmod\bin\fmodstudio.dll .\_bin\
)