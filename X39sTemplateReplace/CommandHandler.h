#pragma once
#include <vector>
#include <string>
#include <math.h>

#define CH_ERROR_OK 1
#define CH_ERROR_COMMANDNOTFOUND -1
#define CH_ERROR_MISSINGARGUMENTS -2
#define CH_ERROR_INVALIDINPUT -3

class CommandHandler;

typedef struct Command
{
	int (*function)(const std::string args[], const int argCount, CommandHandler* cmdHandler);
	std::string functionName;
	std::string description;
	bool hasArguments;
	Command(int (*f)(const std::string args[], const int argCount, CommandHandler* cmdHandler) = NULL, std::string fn = "", std::string desc = "", bool b = false): function(f), functionName(fn), description(desc), hasArguments(b) {}
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
	bool isCommand(char* str);
	COMMAND* parseCommand(const char* str);
	int executeCommand(COMMAND* cmd, const char* str);
	int executeCommand(const char* str);
	static double convAsciiCharToDouble(const char* s, const double fallback);
	static bool convAsciiCharToDouble(const char* s, double* out);
	const COMMAND* getCommands();
	const COMMAND getCommand(int index);
	const int getCommandCount();
};