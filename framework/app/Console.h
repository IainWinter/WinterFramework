#pragma once

#include "Log.h"
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <sstream>

std::vector<std::string> split(const std::string& s, char delim);

enum ConsoleArgType
{
	UNKNOWN, INT, FLOAT, STRING
};

const char* get_console_arg_type_name(ConsoleArgType arg);

struct ConsoleArg
{
	ConsoleArgType type;

	int as_int;
	float as_float;
	std::string as_string; // union was annoying
};

struct ConsoleCommand
{
private:
	std::string verb;
	std::vector<ConsoleArg> args;

public:
	ConsoleCommand(const std::string& arg, const std::vector<ConsoleArg>& args);
	int GetInt(int index) const;
	float GetFloat(int index) const;
	const std::string& GetString(int index) const;

	bool Is(int index, ConsoleArgType type) const;
};

using HandleCommandFunc = std::function<void(const ConsoleCommand&)>;

struct Console
{
private:
	std::unordered_map<std::string, HandleCommandFunc> m_knownCommmands;

public:
	void RegCommand(std::string verb, const HandleCommandFunc& func);
	void Execute(const std::string& commandStr);
};

struct event_ConsoleCommand
{
	std::string command;
};