#pragma once

extern "C" void __declspec(dllexport) setup();

#ifndef IW_NOLOOP
extern "C" bool __declspec(dllexport) loop();
#endif

int main()
{
	setup();

#ifndef IW_NOLOOP
	while (loop()) {}
#endif

	return 0;
}