#pragma once

#include <deque>
#include <string>
#include <sstream>

const std::deque<const char*>& get_all_logs();
std::string combine_all_logs();
void set_max_log_count(size_t count);

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