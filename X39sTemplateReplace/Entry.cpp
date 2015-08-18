#include <string>
#include "CommandHandler.h"
#include "global.hpp"
#include "dotX39\Node.h"
#include "dotX39\DataString.h"
#include "Exceptions\InvalidTypeStructure.h"
#include "XTR\Ressource.hpp"
#include "XTR\Template.hpp"
#include "XTR\Replacement.hpp"

#include <iostream>
#include <sys/stat.h>
#include <Windows.h>
#include <locale>
#include <codecvt>

int createDirectory(std::string path)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	std::wstring wStringPath = converter.from_bytes(path);
	auto result = CreateDirectory(wStringPath.c_str(), NULL);
	if (result == ERROR_PATH_NOT_FOUND)
	{
		createDirectory(path.substr(0, path.rfind('\\')));
		return createDirectory(path);
	}
	return result;
}
int createDirectory(std::wstring path)
{
	auto result = CreateDirectory(path.c_str(), NULL);
	if (result == ERROR_PATH_NOT_FOUND)
	{
		createDirectory(path.substr(0, path.rfind('\\')));
		return createDirectory(path);
	}
	return result;
}

void parse_Ressources(const dotX39::Node& node, std::vector<XTR::Ressource>& ressourceList)
{
	for (int i = 0; i < node.getArgumentCount(); i++)
	{
		auto arg = node.getArgument(i);
		if (arg->getName().compare("basePath") == 0)
		{
			if (arg->getType() != dotX39::DataTypes::STRING)
			{
				std::cout << std::string("the nodes '").append(node.getName()).append("' 'basePath' argument should be STRING") << std::endl;
				throw Exceptions::InvalidTypeStructureException();
			}
			Globals::getInstance().basePath.ressources = static_cast<const dotX39::DataString*>(arg)->getDataAsString();
		}
		else
		{
			if (Globals::getInstance().verbosity)
				std::cout << std::string("the node '").append(node.getName()).append("' contains the unknown argument '").append(arg->getName()).append("', ignored.") << std::endl;
		}
	}
	for (int i = 0; i < node.getNodeCount(); i++)
	{
		const dotX39::Node* node = node->getNode(i);
		ressourceList.push_back(XTR::Ressource::parse(*node));
	}
}
void parse_Templates(const dotX39::Node& node, std::vector<XTR::Template>& templateList, const std::vector<XTR::Ressource>& ressourceList)
{
	for (int i = 0; i < node.getArgumentCount(); i++)
	{
		auto arg = node.getArgument(i);
		if (arg->getName().compare("basePath") == 0)
		{
			if (arg->getType() != dotX39::DataTypes::STRING)
			{
				std::cerr << std::string("the nodes '").append(node.getName()).append("' 'basePath' argument should be STRING") << std::endl;
				throw Exceptions::InvalidTypeStructureException();
			}
			Globals::getInstance().basePath.templates = static_cast<const dotX39::DataString*>(arg)->getDataAsString();
		}
		else
		{
			if (Globals::getInstance().verbosity)
				std::cout << std::string("the node '").append(node.getName()).append("' contains the unknown argument '").append(arg->getName()).append("', ignored.") << std::endl;
		}
	}
	for (int i = 0; i < node.getNodeCount(); i++)
	{
		const dotX39::Node* node = node->getNode(i);
		templateList.push_back(XTR::Template::parse(*node, ressourceList));
	}
}
void parse_Replacements(const dotX39::Node& node, std::vector<XTR::Replacement>& replacementList, const std::vector<XTR::Template>& templateList, const std::vector<XTR::Ressource>& ressourceList)
{
	for (int i = 0; i < node.getArgumentCount(); i++)
	{
		auto arg = node.getArgument(i);
		if (arg->getName().compare("basePath") == 0)
		{
			if (arg->getType() != dotX39::DataTypes::STRING)
			{
				std::cerr << std::string("the nodes '").append(node.getName()).append("' 'basePath' argument should be STRING") << std::endl;
				throw Exceptions::InvalidTypeStructureException();
			}
			Globals::getInstance().basePath.templates = static_cast<const dotX39::DataString*>(arg)->getDataAsString();
		}
		else
		{
			if (Globals::getInstance().verbosity)
				std::cout << std::string("the node '").append(node.getName()).append("' contains the unknown argument '").append(arg->getName()).append("', ignored.") << std::endl;
		}
	}
	for (int i = 0; i < node.getNodeCount(); i++)
	{
		const dotX39::Node* node = node->getNode(i);
		replacementList.push_back(XTR::Replacement::parse(*node, templateList, ressourceList));
	}
}

int main(int argc, char** argv)
{
	CommandHandler cmdHandler = CommandHandler('-');
	std::string argument;
	bool verbosity = false;

	cmdHandler.registerCommand(COMMAND([](const std::vector<std::string>& args, const CommandHandler& cmdHandler)
	{
		std::cout << "Help page:" << std::endl;
		for (int i = 0; i < cmdHandler.getCommandCount(); i++)
		{
			COMMAND cmd = cmdHandler.getCommand(i);
			std::cout << "\t" << cmd.functionName << "\t" << cmd.description << std::endl;
		}
	}, "?", "Outputs an overview of available commands", false));
	cmdHandler.registerCommand(COMMAND([](const std::vector<std::string>& args, const CommandHandler& cmdHandler)
	{
		if (args.size() != 1)
		{
			std::cout << "To many arguments" << std::endl;
			throw std::exception("Invalid Input, too many arguments");
		}
		auto& path = args[0];
		char pathSplitter = '/';
		if (path.find('\\') >= 0)
			pathSplitter = '\\';
		Globals::getInstance().projectFile.path = path.substr(0, path.rfind(pathSplitter) + 1);
		Globals::getInstance().projectFile.name = path.substr(path.rfind(pathSplitter) + 1);

		//Check if file is existing
		struct stat buffer;
		if (stat(path.c_str(), &buffer) != 0)
		{
			std::cout << "The file '" << path << "' is not existing, make sure you provide the absolut path or just the file name (when its in executable dir)" << std::endl;
			Globals::getInstance().projectFile.clear();
			throw std::exception("Invalid Input, file is not existing");
		}
		std::cout << "Path set" << std::endl;
	}, "path", "Input argument that has to point to a project.x39 file", true));
	cmdHandler.registerCommand(COMMAND([](const std::vector<std::string>& args, const CommandHandler& cmdHandler)
	{
		Globals::getInstance().verbosity = true;
		std::cout << "Verbosity enabled" << std::endl;
	}, "verbose", "Enables extended output", false));
	cmdHandler.registerCommand(COMMAND([](const std::vector<std::string>& args, const CommandHandler& cmdHandler)
	{
		std::cout << "About X39s Template-Replace (short: XTR)" << std::endl
			<< "\tXTR was developed by X39 (or without nickname: Marco Silipo)." << std::endl << std::endl
			<< "\tIt is 100% allowed to be used by open-source projects or" << std::endl << "\tcompanies (as long as the company is planning to use whatever was" << std::endl << "\tgenerated with this tool just internally)" << std::endl << std::endl
			<< "\tSpecial note for companies:" << std::endl
			<< "\tIf you plan to release whatever you generate with this" << std::endl
			<< "\tplease contact me before so its possible to make sure" << std::endl
			<< "\tyou dont violate that rule" << std::endl
			<< "\tWhen you plan to make profit with this we also can find" << std::endl
			<< "\ta solution ( just ask kindly :) )" << std::endl
			<< "\tGreets X39" << std::endl;
	}, "about", "Prints informations about the author and 'license'", false));
	cmdHandler.registerCommand(COMMAND([](const std::vector<std::string>& args, const CommandHandler& cmdHandler)
	{
		std::cout << "Version: " << Globals::getInstance().version
			#ifdef _DEBUG
			<< " DEBUG"
			#endif
			<< std::endl;
	}, "version", "Prints current version number", false));
	if (argc == 1)
	{
		std::cout << "No Arguments provided, use -? for a basic help page" << std::endl;
		return -1;
	}
	argument = argv[1];
	for (int i = 2; i < argc; i++)
	{
		if (argv[i][0] != '-')
			if (strchr(argv[i], ' ') == 0)
				argument.append(" ").append(argv[i]);
			else
				argument.append(" \"").append(argv[i]).append("\"");
		else
		{
			cmdHandler.executeCommand(argument.c_str());
			argument = argv[i];
		}
	}
	cmdHandler.executeCommand(argument.c_str());
	//ToDo: Parse
}