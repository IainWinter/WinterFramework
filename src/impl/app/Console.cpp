#include "app/Console.h"

#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <sstream>

std::vector<std::string> split(const std::string &s, char delim)
{
	std::vector<std::string> out;
	std::string working;

	const char* itr = s.c_str();
	const char* end = itr + s.size();

	bool inQuote = false;

	while (itr != end)
	{
		char cur = *itr;
		
		switch (cur)
		{
			case ' ':
				if (!inQuote)
				{
					out.push_back(std::move(working));
					working = {}; // redundant?
				}

				break;

			case '\'':
				inQuote = !inQuote; // should test what too many quotes ends up doing
				break;

			default:
				working.push_back(cur);
				break;
		}

		++itr;
	}

	out.push_back(std::move(working));
	return out;
}

const char* get_console_arg_type_name(ConsoleArgType arg)
{
	switch (arg)
	{
		case ConsoleArgType::NUMBER: return "a number";
		case ConsoleArgType::STRING: return "a string";
	}

	return "unknown";
}

ConsoleCommand::ConsoleCommand(const std::string& arg, const std::vector<ConsoleArg>& args)
	: verb(verb)
	, args(args)
{}

int ConsoleCommand::GetInt(int index) const
{
	if (args.size() <= index)
	{
		log_console("Error arg index out of bounds");
		return 0;
	}

	const ConsoleArg& arg = args.at(index);

	if (arg.type != ConsoleArgType::NUMBER)
	{
		log_console("Arg at index %d is not an int, it is %s", index, get_console_arg_type_name(arg.type));
		return 0;
	}

	return (int)arg.as_number;
}

float ConsoleCommand::GetFloat(int index) const
{
	if (args.size() <= index)
	{
		log_console("Error arg index out of bounds\n");
		return 0.f;
	}

	const ConsoleArg& arg = args.at(index);

	if (arg.type != ConsoleArgType::NUMBER)
	{
		log_console("Arg at index %d is not a float, it is %s", index, get_console_arg_type_name(arg.type));
		return 0.f;
	}

	return (float)arg.as_number;
}

const std::string empty = "";

const std::string& ConsoleCommand::GetString(int index) const
{
	if (args.size() <= index)
	{
		log_console("Error arg index out of bounds");
		return empty;
	}

	const ConsoleArg& arg = args.at(index);

	if (arg.type != ConsoleArgType::STRING)
	{
		log_console("Arg at index %d is not a string, it is %s", index, get_console_arg_type_name(arg.type));
		return empty;
	}

	return arg.as_string;
}

bool ConsoleCommand::Is(int index, ConsoleArgType type) const
{
	return args.size() > index
		&& args.at(index).type == type;
}

void Console::RegCommand(std::string verb, const HandleCommandFunc& func)
{
	m_knownCommmands.emplace(verb, func);
}

void Console::Execute(const std::string& commandStr)
{
	// parse args
	// find command
	// exe action 

	std::vector<std::string> args = split(commandStr, ' ');

	auto itr = m_knownCommmands.find(args[0]);

	if (itr == m_knownCommmands.end())
	{
		log_console("Unknown command '%s'", args[0].c_str());
		return;
	}

	std::vector<ConsoleArg> consoleArgs;

	for (int i = 1; i < args.size(); i++)
	{
		ConsoleArg arg;
			 
		try // ew
		{
			arg.as_number = std::stof(args[i]);
			arg.type = ConsoleArgType::NUMBER;
		} 
		catch (std::exception e)
		{
			try
			{
				arg.as_number = (float)std::stoi(args[i]);
				arg.type = ConsoleArgType::NUMBER;
			}
			catch (std::exception e) 
			{
				try
				{
					arg.as_string = args[i];
					arg.type = ConsoleArgType::STRING;
				}
				catch (std::exception e)
				{
					log_console("Failed to parse arg correctly '%s'", args.at(i).c_str());
					continue;
				}
			}
		}

		consoleArgs.push_back(arg);
	}

	log_console("executing '%s'", commandStr.c_str());

	itr->second(ConsoleCommand(args.at(0), consoleArgs));
}