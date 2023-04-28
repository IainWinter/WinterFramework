#pragma once

enum log_mask : size_t
{
	LOG_AUDIO     = 1 << 0,
	LOG_ENTITY    = 1 << 1,
	LOG_EVENT     = 1 << 2,
	LOG_PHYSICS   = 1 << 3,
	LOG_RENDER    = 1 << 4,
	LOG_WINDOW    = 1 << 5,
	LOG_WORLD     = 1 << 6,
	LOG_CONSOLE   = 1 << 7,
	LOG_IO        = 1 << 8,
	LOG_GAME      = 1 << 9,
	LOG_APP       = 1 << 10,

	LOG_ALL       = 0xFFFFFFFF
};

// enables logging for the flipped bits in the mask, see @log_mask for options
//
void set_log_mask(size_t mask);

// you can write a~ to write the contents of the string 'effect' at flag

// when a log is submitted with a flag, should it be logged
//
void set_log_enabled(char flag, bool enabled);

// defaults are 
//
//   error: e~ red text
//   warn:  w~ yellow text
//   info:  i~ dark gray
//   debug: d~ cyan

// enable or disable styles
//
void set_log_style_flags_enabled(bool enable);

// set the style string for a single char flag
//                    text          highlight
// 
// Black			\033[30m		\033[40m	
// Red				\033[31m		\033[41m
// Green			\033[32m		\033[42m
// Yellow			\033[33m		\033[43m
// Blue				\033[34m		\033[44m
// Magenta			\033[35m		\033[45m
// Cyan				\033[36m		\033[46m
// Light gray		\033[37m		\033[47m
// Dark gray		\033[90m		\033[100m
// Light red		\033[91m		\033[101m
// Light green		\033[92m		\033[102m
// Light yellow		\033[93m		\033[103m
// Light blue		\033[94m		\033[104m
// Light magenta	\033[95m		\033[105m
// Light cyan 		\033[96m		\033[106m
// White			\033[97m		\033[107m
// Bold				\033[1m		       -
// Underline		\033[4m		       -
// No underline		\033[24m           -
// Reversed bg/fg	\033[7m            -
// Positive bg/fg   \033[27m           -
// Default			\033[0m            -
//
void set_log_style_flag_effect(char flag, const char* effect);

void log        (const char* fmt, ...);
void log_audio  (const char* fmt, ...);
void log_entity (const char* fmt, ...);
void log_event  (const char* fmt, ...);
void log_physics(const char* fmt, ...);
void log_render (const char* fmt, ...);
void log_window (const char* fmt, ...);
void log_world  (const char* fmt, ...);
void log_console(const char* fmt, ...);
void log_io     (const char* fmt, ...);
void log_game   (const char* fmt, ...);
void log_app    (const char* fmt, ...);