#include "CommandHandler.h"
#include <iostream>
#include "dotX39\DocumentReader.h"


using namespace std;

static std::string path;
static bool verbose;

int cmdHelp(const std::string args[], const int argCount, CommandHandler* cmdHandler)
{
	cout << "Help page:" << endl;
	for (int i = 0; i < cmdHandler->getCommandCount(); i++)
	{
		COMMAND cmd = cmdHandler->getCommand(i);
		cout << "\t" << cmd.functionName << "\t" << cmd.description << endl;
	}
	return CH_ERROR_OK;
}
int cmdPath(const std::string args[], const int argCount, CommandHandler* cmdHandler)
{
	if (argCount != 1)
	{
		cout << "To many arguments" << endl;
		return CH_ERROR_INVALIDINPUT;
	}
	path = args[0];
	cout << "Path set" << endl;
	return CH_ERROR_OK;
}
int cmdVerbose(const std::string args[], const int argCount, CommandHandler* cmdHandler)
{
	verbose = true;
	cout << "Verbosity enabled" << endl;
	return CH_ERROR_OK;
}
int main(int argc, char* argv[])
{
	path = "";
	verbose = false;
	CommandHandler cmdHandler = CommandHandler('-');
	cmdHandler.registerCommand(COMMAND(cmdHelp, "?", "Outputs an overview of available commands", false));
	cmdHandler.registerCommand(COMMAND(cmdPath, "path", "Input argument that has to point to a project.x39 file", true));
	cmdHandler.registerCommand(COMMAND(cmdVerbose, "v", "Enables extended output", false));
	if (argc == 1)
	{
		cout << "No Arguments provided, use -? for a basic help page" << endl;
		system("pause");
		exit(-1);
	}
	string arg = argv[1];
	for (int i = 2; i < argc; i++)
	{
		if (argv[i][0] != '-')
			arg.append(" ").append(argv[i]);
		else
		{
			cmdHandler.executeCommand(arg.c_str());
			arg = argv[i];
		}
	}
	cmdHandler.executeCommand(arg.c_str());
	if (path.empty())
	{
		cout << "No path given, please provide a path argument" << endl;
		system("pause");
		exit(-1);
	}
	dotX39::Node* node = new dotX39::Node("root");
	dotX39::DocumentReader::readDocument(path, node);

	dotX39::Node* templates = NULL;
	dotX39::Node* replacements = NULL;

	delete node;
	system("pause");
	exit(1);
}