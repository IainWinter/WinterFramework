#include "Log.h"

#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <unordered_map>

static bool g_enable_style = true;

static size_t g_mask = LOG_ALL;

static std::unordered_map<char, const char*> g_styles =
{
	{'e', "\033[91m"},
	{'w', "\033[93m"},
	{'i', "\033[90m"},
	{'d', "\033[96m"}
};

static std::unordered_map<char, bool> g_enabled =
{
	{'e', true},
	{'w', true},
	{'i', true},
	{'d', true}
};

void set_log_mask(size_t mask)
{
	g_mask = mask;
}

void set_log_enabled(char flag, bool enabled)
{
	g_enabled[flag] = enabled;
}

void set_log_style_flags_enabled(bool enable)
{
	g_enable_style = enable;
}

void set_log_style_flag_effect(char flag, const char* effect)
{
	g_styles[flag] = effect;
}

void submit_log(const char* str)
{
	printf("%s", str);
}

void put(char** str_itr, char c)
{
	**str_itr = c;
	*str_itr += 1;
}

size_t put_str(char** str_itr, const char* str, char stop_char = '\0')
{
	const char* start = str;

	while (*str != stop_char && *str != '\0') // weird but need to make sure it's not passed end anyway
	{
		put(str_itr, *str);
		str += 1;
	}

	return (size_t)str - (size_t)start + 1;
}

void log_with_header(const char* header, const char* fmt, va_list args)
{
	size_t len = strlen(fmt);

	if (len == 0)
		return;

	const char* text = fmt;
	char style_token = fmt[0];
	bool has_style = len >= 2 && fmt[1] == '~';

	if (has_style) // log includes style token
	{
		text += 2; // skip the format even if styles are disabled

		// if a style doesn't exist for this token
		// or it's not enabled

		if (g_enabled.count(style_token) == 0 || !g_enabled.at(style_token))        
			return; // exit

		// if style token, but no style

		if (g_styles.count(style_token) == 0)
			return; // exit
	}

	// now we create the log and print it

	char str[1024 * 5];
	char* itr = str;

	if (has_style && g_enable_style) // if styles are enabled, write the style
		put_str(&itr, g_styles.at(style_token));

	put_str(&itr, header);

	if (strchr(text, '\n') == 0) // if there are no newlines, set formatted string directly
	{
		vsprintf(itr, text, args);
		itr += strlen(itr);
	}

	else // or write to a temp buffer for newline tabbing                             ... todo: there is some issue with the chars after tmp_itr printing random bytes
	{
		char tmp[1024 * 5];
		const char* tmp_itr = tmp;

		vsprintf(tmp, text, args);
		tmp_itr += put_str(&itr, tmp_itr, '\n');

		while (*tmp_itr != '\0')
		{
			put(&itr, '\n'); // put newline

			for (int i = 0; i < strlen(header); i++) // write spaces
				put(&itr, ' ');

			tmp_itr += put_str(&itr, tmp_itr, '\n');
		}
	}

	if (text != fmt) // set to default if there was a style specified and it was used
	{
		put(&itr, '\033');
		put(&itr, '[');
		put(&itr, 'm');
	}

	put(&itr, '\n'); // put newline
	put(&itr, '\0'); // put end string

	submit_log(str);
}

#define DO_LOG(header) { va_list args; va_start(args, fmt); log_with_header(header, fmt, args); va_end(args); }

void log        (const char* fmt, ...) {                           DO_LOG("          ") }
void log_audio  (const char* fmt, ...) { if (g_mask & LOG_AUDIO)   DO_LOG("[Audio  ] ") }
void log_entity (const char* fmt, ...) { if (g_mask & LOG_ENTITY)  DO_LOG("[Entity ] ") }
void log_event  (const char* fmt, ...) { if (g_mask & LOG_EVENT)   DO_LOG("[Event  ] ") }
void log_physics(const char* fmt, ...) { if (g_mask & LOG_PHYSICS) DO_LOG("[Physics] ") }
void log_render (const char* fmt, ...) { if (g_mask & LOG_RENDER)  DO_LOG("[Render ] ") }
void log_window (const char* fmt, ...) { if (g_mask & LOG_WINDOW)  DO_LOG("[Window ] ") }
void log_world  (const char* fmt, ...) { if (g_mask & LOG_WORLD)   DO_LOG("[World  ] ") }
void log_console(const char* fmt, ...) { if (g_mask & LOG_CONSOLE) DO_LOG("[Console] ") }
void log_io     (const char* fmt, ...) { if (g_mask & LOG_IO)      DO_LOG("[IO     ] ") }
void log_game   (const char* fmt, ...) { if (g_mask & LOG_GAME)    DO_LOG("[Game   ] ") }
void log_app    (const char* fmt, ...) { if (g_mask & LOG_APP)     DO_LOG("[App    ] ") }

#undef DO_LOG