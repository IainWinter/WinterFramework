#include "Log.h"

#include <stdarg.h>
#include <string.h>

namespace wlog
{
	wContextImpl(log_context);
}

#define ctx wlog::ctx

void set_max_log_count(size_t count)
{
	ctx->m_max_size = count;
}

void set_log_mask(size_t mask)
{
	ctx->m_mask = mask;
}

void set_log_enabled(char flag, bool enabled)
{
	ctx->m_enabled[flag] = enabled;
}

void set_log_style_flags_enabled(bool enable)
{
	ctx->m_enable_style = enable;
}

void set_log_style_flag_effect(char flag, const char* effect)
{
	ctx->m_styles[flag] = effect;
}

void submit_log(const char* str)
{
	if (ctx->m_record.size() > ctx->m_max_size)
	{
		ctx->m_pool.free_bytes((void*)ctx->m_record.front(), wLOG_SIZE);
		ctx->m_record.pop_front();
	}

	ctx->m_record.push_back(str);

	printf("%s", str);
}

char* get_log_mem()
{
	char* str = ctx->m_pool.alloc_bytes(wLOG_SIZE);
	memset(str, 0, wLOG_SIZE);
	return str;
}

void put(char** str, char c)
{
	**str = c;
	*str += 1;
}

void log_with_header(const char* header, const char* fmt, va_list args)
{
	char* str = get_log_mem();
	char* itr = str;

	const char* text = fmt;

	if (strlen(text) > 1 && text[1] == '~') // set style or exit if disabled, default is enabled
	{
		if (ctx->m_enable_style)
		{
			char flag = text[0];
		
			auto enabled = ctx->m_enabled.find(flag);
			if (enabled != ctx->m_enabled.end())
			{
				if (!enabled->second) return; // exit
			}

			auto style = ctx->m_styles.find(flag);
			if (style != ctx->m_styles.end())
			{
				const char* itr_style = style->second;

				while (*itr_style != '\0')
				{
					put(&itr, *itr_style);
					itr_style += 1;
				}
			}
		}

		text += 2; // dont write style flag
	}

	int header_length = strlen(header);

	if (header_length > 0) // write header
	{
		put(&itr, '[');

		while (*header != '\0')
		{
			put(&itr, *header);
			header += 1;
		}

		put(&itr, ']');
		put(&itr, ' ');
	}

	// tab over newlines
	//while (*text != '\0')
	//{
	//	put(&itr, *text);

	//	if (*text == '\n')
	//	{
	//		for (int i = 0; i < header_length; i++)
	//		{
	//			put(&itr, ' ');
	//		}
	//	}

	//	text += 1;
	//}

	vsprintf(itr, text, args); // write text
	itr += strlen(itr);

	if (text != fmt) // set to default if there was a style specified and it was used
	{
		put(&itr, '\033');
		put(&itr, '[');
		put(&itr, 'm');
	}

	put(&itr, '\n'); // put newline

	submit_log(str);
}

#define DO_LOG(header) { va_list args; va_start(args, fmt); log_with_header(header, fmt, args); va_end(args); }

void log        (const char* fmt, ...) {                                DO_LOG("")        }
void log_audio  (const char* fmt, ...) { if (ctx->m_mask & LOG_AUDIO)   DO_LOG("Audio  ") }
void log_entity (const char* fmt, ...) { if (ctx->m_mask & LOG_ENTITY)  DO_LOG("Entity ") }
void log_event  (const char* fmt, ...) { if (ctx->m_mask & LOG_EVENT)   DO_LOG("Event  ") }
void log_physics(const char* fmt, ...) { if (ctx->m_mask & LOG_PHYSICS) DO_LOG("Physics") }
void log_render (const char* fmt, ...) { if (ctx->m_mask & LOG_RENDER)  DO_LOG("Render ") }
void log_window (const char* fmt, ...) { if (ctx->m_mask & LOG_WINDOW)  DO_LOG("Window ") }
void log_world  (const char* fmt, ...) { if (ctx->m_mask & LOG_WORLD)   DO_LOG("World  ") }
void log_console(const char* fmt, ...) { if (ctx->m_mask & LOG_CONSOLE) DO_LOG("Console") }
void log_io     (const char* fmt, ...) { if (ctx->m_mask & LOG_IO)      DO_LOG("IO     ") }
void log_game   (const char* fmt, ...) { if (ctx->m_mask & LOG_GAME)    DO_LOG("Game   ") }
void log_app    (const char* fmt, ...) { if (ctx->m_mask & LOG_APP)     DO_LOG("App    ") }
