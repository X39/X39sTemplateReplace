#include <string>
#include "CommandHandler.h"
#include "global.hpp"
#include "dotX39\Node.h"
#include "dotX39\DataString.h"
#include "dotX39\DocumentReader.h"
#include "Exceptions\InvalidTypeStructure.h"
#include "XTR\Ressource.hpp"
#include "XTR\Template.hpp"
#include "XTR\Replacement.hpp"

#include <iostream>
#include <sys/stat.h>
#include <Windows.h>
#include <locale>
#include <codecvt>
#include <direct.h>

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
		const dotX39::Node* subnode = node.getNode(i);
		auto tmp = XTR::Ressource::parse(*subnode);
		ressourceList.push_back(tmp);
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
		const dotX39::Node* subnode = node.getNode(i);
		templateList.push_back(XTR::Template::parse(*subnode, ressourceList));
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
		const dotX39::Node* subnode = node.getNode(i);
		replacementList.push_back(XTR::Replacement::parse(*subnode, templateList, ressourceList));
	}
}

void writeFile(XTR::Replacement r)
{
	if (Globals::getInstance().verbosity)
		std::cout << "Creating directory " << r.getOutputFolder() << "' if it not exists" << std::endl;
	createDirectory(r.getOutputFolder());
	for (auto& it : r.getRessources())
		CopyFileA(it.getFilePath().c_str(), std::string(r.getOutputFolder()).append(it.getOutputFileName()).c_str(), false);
	for (auto& it : r.getTemplates())
	{
		std::vector<XTR::TemplateKeyword> templateKeywords;
		for (auto& it2 : it.getRessources())
			CopyFileA(it2.getFilePath().c_str(), std::string(r.getOutputFolder()).append(it2.getOutputFileName()).c_str(), false);
		//Replace TemplateKeywords in the replacement
		
		for (auto& tk : r.getTemplateKeywords())
		{
			std::string str = tk.getText();
			//bool flag = false;
			for (auto& rk : it.getKeywords().replacements)
			{
				int findResult = str.find(rk.getAppeariance());
				if (findResult != -1)
				{
					//Insert replacement
					while (findResult != -1)
					{
						str = std::string(str.substr(0, findResult)).append(rk.getReplacement()).append(str.substr(findResult + rk.getAppeariance().length()));
						findResult = str.find(rk.getAppeariance());
					}
					//flag = true;
				}
			}
			templateKeywords.push_back(XTR::TemplateKeyword(tk.getQualifier(), str));
			//else if (Globals::getInstance().verbosity)
			//	std::cout << "The Keyword '" << tk.getQualifier() << "' uses no replacement keywords" << std::endl;
		}
		std::string fileName = std::string(it.getOutputFileName()).append(".").append(it.getOutputFileExtension());
		if (Globals::getInstance().verbosity)
			std::cout << "Creating file '" << fileName << "' from template '" << it.getQualifier() << "'" << std::endl;
		std::ofstream stream = std::ofstream(std::string(r.getOutputFolder()).append(fileName));
		std::string content = it.getFileData();
		//for (int j = 0; j < t->keywordsTemplate.size(); j++)
		for (auto& templateKeywordT : it.getKeywords().templates)
		{
			for (auto& templateKeywordR : templateKeywords)
			{
				if (templateKeywordR.getQualifier().compare(templateKeywordT.getQualifier()) == 0)
				{
					int findResult = content.find(templateKeywordT.getText());
					if (findResult == -1)
					{
						if (Globals::getInstance().verbosity)
							std::cout << "Template " << it.getQualifier() << " has the unused keyword '" << templateKeywordT.getQualifier() << "'" << std::endl;
					}
					else
					{
						do {
							//Insert replacement
							content = std::string(content.substr(0, findResult)).append(templateKeywordR.getText()).append(content.substr(findResult + templateKeywordT.getText().length()));
						} while ((findResult = content.find(templateKeywordT.getText())) != -1);
					}
					break;
				}
			}
		}
		stream.write(content.c_str(), content.size());
		stream.close();
	}
}

int main(int argc, char** argv)
{
	CommandHandler cmdHandler = CommandHandler('-');
	std::string argument;
	dotX39::Node* documentRoot;
	struct {
		const dotX39::Node* templates;
		const dotX39::Node* ressources;
		const dotX39::Node* replacements;
	} baseNode;
	std::vector<XTR::Ressource> ressourceList;
	std::vector<XTR::Template> templateList;
	std::vector<XTR::Replacement> replacementList;
#pragma region Command Registering
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
		Globals::getInstance().exitAfterArgReading = false;
		_chdir(Globals::getInstance().projectFile.path.c_str());

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
#pragma endregion
#pragma region Argument Parsing
	if (argc == 1)
	{
		std::cout << "No Arguments provided, use -? for a basic help page" << std::endl;
		return -1;
	}
	argument = argv[1];
	try
	{
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
	}
	catch (std::exception e)
	{
		std::cerr << "Error while reading commands:" << std::endl << e.what() << std::endl;
		return -1;
	}
	
	if (Globals::getInstance().exitAfterArgReading)
		return 0;
#pragma endregion
#pragma region read dotX39 document
	documentRoot = new dotX39::Node("root");
	try
	{
		dotX39::DocumentReader::readDocument(std::string(Globals::getInstance().projectFile.path).append(Globals::getInstance().projectFile.name), documentRoot);
	}
	catch (std::exception e)
	{
		std::cout << "Ran into problems while reading the project file :(" << std::endl;
		std::cout << e.what() << std::endl;
		return -1;
	}
#pragma endregion
#pragma region Receive BasePath from dotX39 nodes
	for (int i = 0; i < documentRoot->getNodeCount(); i++)
	{
		const dotX39::Node* node = documentRoot->getNode(i);
		if (node->getName().compare("templates") == 0)
		{
			baseNode.templates = node;
			for (int j = 0; j < node->getArgumentCount(); j++)
			{
				const dotX39::Data* arg = node->getArgument(j);
				if (arg->getName().compare("basePath") == 0)
				{
					if (arg->getType() != dotX39::DataTypes::STRING)
					{
						std::cout << std::string("the nodes '").append(node->getName()).append("' basePath argument should be STRING") << std::endl;
						delete documentRoot;
						return -1;
					}
					Globals::getInstance().basePath.templates = std::string(Globals::getInstance().projectFile.path).append(((dotX39::DataString*)arg)->getDataAsString());
				}
				else
				{
					if (Globals::getInstance().verbosity)
						std::cout << std::string("the node '").append(node->getName()).append("' contains the unknown argument '").append(arg->getName()).append("', ignored.") << std::endl;
				}
			}
		}
		else if (node->getName().compare("ressources") == 0)
		{
			baseNode.ressources = node;
			for (int j = 0; j < node->getArgumentCount(); j++)
			{
				const dotX39::Data* arg = node->getArgument(j);
				if (arg->getName().compare("basePath") == 0)
				{
					if (arg->getType() != dotX39::DataTypes::STRING)
					{
						std::cout << std::string("the nodes '").append(node->getName()).append("' basePath argument should be STRING") << std::endl;
						delete documentRoot;
						return -1;
					}
					Globals::getInstance().basePath.ressources = std::string(Globals::getInstance().projectFile.path).append(((dotX39::DataString*)arg)->getDataAsString());
				}
				else
				{
					if (Globals::getInstance().verbosity)
						std::cout << std::string("the node '").append(node->getName()).append("' contains the unknown argument '").append(arg->getName()).append("', ignored.") << std::endl;
				}
			}
		}
		else if (node->getName().compare("replacements") == 0)
		{
			baseNode.replacements = node;
			for (int j = 0; j < node->getArgumentCount(); j++)
			{
				const dotX39::Data* arg = node->getArgument(j);
				if (arg->getName().compare("basePath") == 0)
				{
					if (arg->getType() != dotX39::DataTypes::STRING)
					{
						std::cout << std::string("the nodes '").append(node->getName()).append("' basePath argument should be STRING") << std::endl;
						delete documentRoot;
						return -1;
					}
					Globals::getInstance().basePath.replacement = std::string(Globals::getInstance().projectFile.path).append(((dotX39::DataString*)arg)->getDataAsString());
				}
				else
				{
					if (Globals::getInstance().verbosity)
						std::cout << std::string("the node '").append(node->getName()).append("' contains the unknown argument '").append(arg->getName()).append("', ignored.") << std::endl;
				}
			}
		}
		else
		{
			if (Globals::getInstance().verbosity)
				std::cout << std::string("the node '").append(node->getName()).append("' is Unknown as base node") << std::endl;
		}
	}
#pragma endregion
#pragma region Parse dotX39 nodes into proper objects
	//Parse Ressources
	if (baseNode.ressources != NULL)
		parse_Ressources(*baseNode.ressources, ressourceList);
	else if (Globals::getInstance().verbosity)
		std::cout << "No 'ressources' node was found" << std::endl;

	//Parse Templates
	if (baseNode.ressources != NULL)
		parse_Templates(*baseNode.templates, templateList, ressourceList);
	else
	{
		std::cerr << "No 'templates' node was found" << std::endl;
		delete documentRoot;
		return -1;
	}

	//Parse Replacements
	if (baseNode.ressources != NULL)
		parse_Replacements(*baseNode.replacements, replacementList, templateList, ressourceList);
	else
	{
		std::cerr << "No 'replacements' node was found" << std::endl;
		delete documentRoot;
		return -1;
	}
	//Delete document as we dont need it anymore
	delete documentRoot;
#pragma endregion
#pragma region Write stuff to file
	for (auto& it : replacementList)
	{
		if (Globals::getInstance().verbosity)
			std::cout << std::endl << std::string(20, '-') << std::endl << "Creating files for " << it.getQualifier() << std::endl;
		else
			std::cout << "Creating files for " << it.getQualifier() << std::endl;
		writeFile(it);
		if (Globals::getInstance().verbosity)
			std::cout << std::string(20, '-') << std::endl;
	}
#pragma endregion
	std::cout << "XTR job completed" << std::endl;
}
