#include "Log.h"
#include "util/pool_allocator.h"

#include <stdarg.h>

#define LOG_SIZE 1024

pool_allocator m_pool = pool_allocator(LOG_SIZE, 2);
std::deque<const char*> m_record;
size_t m_max_size = 1000;

const std::deque<const char*>& get_all_logs()
{
	return m_record;
}

std::string combine_all_logs()
{
	std::stringstream ss;
	for (const char* str : get_all_logs())
	{
		ss << str;
	}

	std::string str = ss.str();
	str.pop_back();

	return str;
}

void set_max_log_count(size_t count)
{
	m_max_size = count;
}

void submit_log(char* str)
{
	if (m_record.size() > m_max_size)
	{
		m_pool.free_bytes((void*)m_record.front(), LOG_SIZE);
		m_record.pop_front();
	}

	m_record.push_back(str);

	printf(str);
}

char* get_log_mem()
{
	char* str = m_pool.alloc_bytes(LOG_SIZE);
	memset(str, 0, LOG_SIZE);
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

	if (strlen(header) > 0)
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

	vsprintf(itr, fmt, args);

	itr += strlen(itr);
	put(&itr, '\n');

	submit_log(str);
}

void log        (const char* fmt, ...) { va_list args; va_start(args, fmt); log_with_header("",        fmt, args); va_end(args); }
void log_audio  (const char* fmt, ...) { va_list args; va_start(args, fmt); log_with_header("Audio",   fmt, args); va_end(args); }
void log_entity (const char* fmt, ...) { va_list args; va_start(args, fmt); log_with_header("Entity",  fmt, args); va_end(args); }
void log_event  (const char* fmt, ...) { va_list args; va_start(args, fmt); log_with_header("Event",   fmt, args); va_end(args); }
void log_physics(const char* fmt, ...) { va_list args; va_start(args, fmt); log_with_header("Physics", fmt, args); va_end(args); }
void log_render (const char* fmt, ...) { va_list args; va_start(args, fmt); log_with_header("Render",  fmt, args); va_end(args); }
void log_window (const char* fmt, ...) { va_list args; va_start(args, fmt); log_with_header("Window",  fmt, args); va_end(args); }
void log_world  (const char* fmt, ...) { va_list args; va_start(args, fmt); log_with_header("World",   fmt, args); va_end(args); }
void log_console(const char* fmt, ...) { va_list args; va_start(args, fmt); log_with_header("Console", fmt, args); va_end(args); }
void log_io     (const char* fmt, ...) { va_list args; va_start(args, fmt); log_with_header("IO",      fmt, args); va_end(args); }
void log_game   (const char* fmt, ...) { va_list args; va_start(args, fmt); log_with_header("Game",    fmt, args); va_end(args); }
void log_app    (const char* fmt, ...) { va_list args; va_start(args, fmt); log_with_header("App",     fmt, args); va_end(args); }