#pragma once

extern void setup();

#ifndef IW_NOLOOP
extern bool loop();
#endif

int main()
{
	setup();

#ifndef IW_NOLOOP
	while (loop()) {}
#endif

	return 0;
}