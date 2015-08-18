#include "commandHandler.h"

CommandHandler::CommandHandler()
{
	this->commandInitiatorChar = NULL;
	this->commands = std::vector<Command>();
}
CommandHandler::CommandHandler(char commandPrefix)
{
	this->commandInitiatorChar = commandPrefix;
	this->commands = std::vector<Command>();
}
CommandHandler::~CommandHandler()
{
}

int CommandHandler::registerCommand(COMMAND cmd)
{
	this->commands.push_back(cmd);
	return 0;
}
bool CommandHandler::isCommand(char* str) const
{
	try
	{
		parseCommand(str);
		return false;
	}
	catch (std::exception e)
	{
		return false;
	}
}
const COMMAND& CommandHandler::parseCommand(const char* str) const
{
	std::string s = std::string(str);
	if ((0 == this->commandInitiatorChar) || (str[0] == this->commandInitiatorChar))
	{
		for (int i = 0; i < this->commands.size(); i++)
		{
			const COMMAND& cmd = this->commands[i];
			int compareResult = s.compare((0 == this->commandInitiatorChar ? 0 : 1), s.find(' ') - (this->commandInitiatorChar == NULL ? 0 : 1), cmd.functionName);
			if (compareResult == 0)
				return cmd;
		}
	}
	throw std::exception("Command is not existing");
}
void CommandHandler::executeCommand(const COMMAND& cmd, const char* str) const
{
	char argument[256];
	int curArgLength = 0;
	int argumentCount = 0;
	bool ignoreFlag = false;
	std::vector<std::string> args = std::vector<std::string>();
	for (int i = 0; i < strlen(str); i++)
	{
		if (str[i] == '"')
		{
			ignoreFlag = !ignoreFlag;
		}
		else
		{
			argument[curArgLength] = str[i];
			curArgLength++;
			if (str[i] == ' ' && !ignoreFlag)
			{
				argumentCount++;
				argument[curArgLength - 1] = (char)0x00;
				args.push_back(std::string(argument));
				curArgLength = 0;
			}
		}
	}
	if (ignoreFlag)
		throw std::exception("Invalid Input");
	argument[curArgLength] = (char)0x00;
	args.push_back(argument);
	if (cmd.hasArguments && argumentCount <= 0)
		throw std::exception("Missing arguments");
	(cmd.function)(args, *this);
}
double CommandHandler::convAsciiCharToDouble(const char* s, const double fallback)
{
	double d = 0;
	if (convAsciiCharToDouble(s, &d))
		return d;
	return fallback;
}

bool CommandHandler::convAsciiCharToDouble(const char* s, double* out)
{
	long int i = 0;
	const char* numbers = NULL;
	int numberLength = -1;
	int dotSpot = -1;
	if (s[0] != 0x00 && s[1] != 0x00 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
	{//HEX
		//Do we got faked here? Validating that AT LEAST 1 HEXA digit is used after the 0x
		for (int index = 2;; index++)
		{
			const char c = s[index];
			if (c == 0x00)
				break;
			if ((c >= 48 && c <= 57) || (c >= 65 && c <= 70) || (c >= 97 && c <= 102))
			{
				numbers = s + index;
				break;
			}
		}
		//Got no HEXA digit? Return false
		if (numbers == NULL)
			return false;
		//Count HEXA digits
		for (int index = 0;; index++)
		{
			const char c = numbers[index];
			if (!((c >= 48 && c <= 57) || (c >= 65 && c <= 70) || (c >= 97 && c <= 102)))
				break;
			numberLength++;
		}
		//Convert HEXA digits to propper integer
		for (int index = numberLength; index >= 0; index--)
		{

			int currentHandleNumber;
			if (((int)numbers[index]) > '9')
			{
				if (((int)numbers[index]) > 'F')
					currentHandleNumber = ((int)numbers[index]) - 'a' + 10;
				else
					currentHandleNumber = ((int)numbers[index]) - 'A' + 10;
			}
			else
			{
				currentHandleNumber = ((int)numbers[index]) - '0';
			}

			i += currentHandleNumber << ((int)(numberLength - index) * 4);
		}
	}
	else
	{//DOUBLE
		//Search for first existing digit (input "qwt 123" will put a pointer to the '1' into numbers)
		for (int index = 0;; index++)
		{
			const char c = s[index];
			if (c == 0x00)
				break;
			if (c >= 48 && c <= 57)
			{
				//found a digit, set pointer & break out of loop
				numbers = s + index;
				break;
			}
		}
		//No digit found? Return false
		if (numbers == NULL)
			return false;
		//Count digits (ignoring the dot)
		for (int index = 0;; index++)
		{
			const char c = numbers[index];
			if ((c < 48 || c > 57) && c != 46)
				break;
			numberLength++;
		}
		//Convert digits into proper scalar numbers 
		for (int index = numberLength; index >= 0; index--)
		{
			int currentHandleNumber = ((int)numbers[index]) - 48;
			if (currentHandleNumber < 0)
			{
				//dot located, we got a float number here!
				dotSpot = numberLength - index;
				numberLength--;
			}
			else
			{
				//use 10^I * X to get the final result
				i += currentHandleNumber * pow((double)10, (int)(numberLength - index));
			}
		}
		//If we had a dot, we will now shift our results comma
		if (dotSpot != -1)
			*out = i / pow((double)10, (int)(dotSpot));
	}
	return true;
}

const std::vector<COMMAND>& CommandHandler::getCommands() const
{
	return this->commands;
}
int CommandHandler::getCommandCount() const
{
	return this->commands.size();
}
const COMMAND& CommandHandler::getCommand(int index) const
{
	return this->commands[index];
}