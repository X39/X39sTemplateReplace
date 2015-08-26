#pragma once
#include <vector>
#include <string>
#include <math.h>

class CommandHandler;

typedef struct Command
{
	void(*function)(const std::vector<std::string>& args, const CommandHandler& cmdHandler);
	std::string functionName;
	std::string description;
	bool hasArguments;
	Command(void(*f)(const std::vector<std::string>& args, const CommandHandler& cmdHandler) = NULL, std::string fn = "", std::string desc = "", bool b = false) : function(f), functionName(fn), description(desc), hasArguments(b) {}
} COMMAND;

class CommandHandler {
private:
	std::vector<COMMAND> commands;
public:
	char commandInitiatorChar;

public:
	CommandHandler();
	CommandHandler(char commandPrefix);
	~CommandHandler();

	int registerCommand(COMMAND cmd);
	bool isCommand(char* str) const;
	const COMMAND& parseCommand(const char* str) const;
	inline void executeCommand(const char* str) const
	{
		return executeCommand(parseCommand(str), str);
	}
	void executeCommand(const COMMAND& cmd, const char* str) const;
	static double convAsciiCharToDouble(const char* s, const double fallback);
	static bool convAsciiCharToDouble(const char* s, double* out);
	const std::vector<COMMAND>& getCommands() const;
	const COMMAND& getCommand(int index) const;
	int getCommandCount() const;
};