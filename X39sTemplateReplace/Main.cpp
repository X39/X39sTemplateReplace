#include "CommandHandler.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "dotX39\DocumentReader.h"
#include "dotX39\DataString.h"
#include "dotX39\DataArray.h"
#include <locale>
#include <codecvt>
#include <Windows.h>
#include <sys/stat.h>

#ifdef _DEBUG
#define exit(FOO) system("pause"); exit(FOO);
#endif

using namespace std;

/** Variables which will be set by command line arguments */
static std::string projectFileName;
static std::string basePath;
static bool verbose;


/** Basic structure containing all informations about the templateKeywords used inside of a template */
typedef struct strTemplateKeyword
{
	string name;
	string text;
} TEMPLATEKEYWORD;
/** Basic structure containing all informations about the replacementKeywords of a template */
typedef struct strReplacementKeyword
{
	string name;
	string keywordAppeariance;
	string keywordReplacement;
} REPLACEMENTKEYWORD;
typedef struct strRessource
{
	string name;
	string filePath;
	string outname;
} RESSOURCE;
/** Basic structure representing a Template */
typedef struct strTemplate
{
	string name;
	string fileName;
	string content;
	string fileExtension;
	string filePath;
	vector<string> usedRessources;
	vector<TEMPLATEKEYWORD> keywordsTemplate;
	vector<REPLACEMENTKEYWORD> keywordsReplace;
} TEMPLATE;

/** Basic structure representing a Template */
typedef struct strReplacement
{
	string name;
	string folderPath;
	vector<string> usedTemplates;
	vector<string> usedRessources;
	vector<TEMPLATEKEYWORD> keywords;
} REPLACEMENT;


#pragma region command definitions
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
int cmdAbout(const std::string args[], const int argCount, CommandHandler* cmdHandler)
{
	cout << "About X39s Template-Replace (short: XTR)" << endl
		<< "\tXTR was developed by X39 (or without nickname: Marco Silipo)." << endl << endl
		<< "\tIt is 100% allowed to be used by open-source projects or" << endl << "\tcompanies (as long as the company is planning to use whatever was" << endl << "\tgenerated with this tool just internally)" << endl << endl
		<< "\tSpecial note for companies:" << endl
		<< "\tIf you plan to release whatever you generate with this" << endl
		<< "\tplease contact me before so its possible to make sure" << endl
		<< "\tyou dont violate that rule" << endl
		<< "\tWhen you plan to make profit with this we also can find" << endl
		<< "\ta solution ( just ask kindly :) )" << endl
		<< "\tGreets X39" << endl;
	return CH_ERROR_OK;
}
int cmdVersion(const std::string args[], const int argCount, CommandHandler* cmdHandler)
{
	cout
		<< "Version: 1.0.0-RC1"
#ifdef _DEBUG
		<< " DEBUG VERSION"
#endif
		<< endl;
	return CH_ERROR_OK;
}
int cmdPath(const std::string args[], const int argCount, CommandHandler* cmdHandler)
{
	if (argCount != 1)
	{
		cout << "To many arguments" << endl;
		return CH_ERROR_INVALIDINPUT;
	}
	char pathSplitter = '/';
	if (args[0].find('\\') >= 0)
		pathSplitter = '\\';
	basePath = args[0].substr(0, args[0].rfind(pathSplitter) + 1);
	projectFileName = args[0].substr(args[0].rfind(pathSplitter) + 1);
	struct stat buffer;
	if (stat(string(basePath).append(projectFileName).c_str(), &buffer) != 0)
	{
		cout << "The file '" << basePath << projectFileName << "' is not existing, make sure you provide the absolut path or just the file name (when its in executable dir)" << endl;
		basePath = "";
		projectFileName = "";
		return CH_ERROR_INVALIDINPUT;
	}
	cout << "Path set" << endl;
	return CH_ERROR_OK;
}
int cmdVerbose(const std::string args[], const int argCount, CommandHandler* cmdHandler)
{
	verbose = true;
	cout << "Verbosity enabled" << endl;
	return CH_ERROR_OK;
}
#pragma endregion
void parseReplacements(vector<REPLACEMENT>& replacementsFiles, const dotX39::Node* replacements, string replacementsBasePath)
{
	//Create replacements structures
	for (int i = 0; i < replacements->getNodeCount(); i++)
	{
		const dotX39::Node* node = replacements->getNode(i);
		REPLACEMENT r;
		r.name = node->getName();
		//Read the arguments into the REPLACEMENT structure
		for (int j = 0; j < node->getArgumentCount(); j++)
		{
			const dotX39::Data* arg = node->getArgument(j);
			if (arg->getName().compare("path") == 0)
			{
				if (arg->getType() != dotX39::DataTypes::STRING)
				{
					cout << string("the nodes '").append(node->getName()).append("' path argument should be STRING") << endl;
					exit(-1);
				}
				r.folderPath = string().append(replacementsBasePath).append(((dotX39::DataString*)arg)->getDataAsString());
			}
			else
			{
				if (verbose)
					cout << string("the node '").append(node->getName()).append("' contains the unknown argument '").append(arg->getName()).append("'") << endl;
			}
		}
		for (int j = 0; j < node->getDataCount(); j++)
		{
			const dotX39::Data* data = node->getData(j);
			if (data->getName().compare("templates") == 0)
			{
				if (data->getType() != dotX39::DataTypes::ARRAY)
				{
					cout << string("the nodes '").append(node->getName()).append("' templates argument should be an ARRAY of strings") << endl;
					exit(-1);
				}
				//Read the templates array and put it into the structure
				for (int k = 0; k < ((dotX39::DataArray*)data)->getDataCount(); k++)
				{
					const dotX39::Data* arrayData = ((dotX39::DataArray*)data)->getDataElement(k);
					if (arrayData->getType() != dotX39::DataTypes::STRING)
					{
						cout << string("the nodes '").append(node->getName()).append("' templates data contains non-string entries") << endl;
						exit(-1);
					}
					r.usedTemplates.push_back(((dotX39::DataString*)arrayData)->getDataAsString());
				}
			}
			else if (data->getName().compare("ressources") == 0)
			{
				if (data->getType() != dotX39::DataTypes::ARRAY)
				{
					cout << string("the nodes '").append(node->getName()).append("' ressources argument should be an ARRAY of strings") << endl;
					exit(-1);
				}
				//Read the templates array and put it into the structure
				for (int k = 0; k < ((dotX39::DataArray*)data)->getDataCount(); k++)
				{
					const dotX39::Data* arrayData = ((dotX39::DataArray*)data)->getDataElement(k);
					if (arrayData->getType() != dotX39::DataTypes::STRING)
					{
						cout << string("the nodes '").append(node->getName()).append("' ressources data contains non-string entries") << endl;
						exit(-1);
					}
					r.usedRessources.push_back(((dotX39::DataString*)arrayData)->getDataAsString());
				}
			}
			else
			{
				if (verbose)
					cout << string("the node '").append(node->getName()).append("' contains the unknown data '").append(data->getName()).append("', ignored.") << endl;
			}
		}
		for (int j = 0; j < node->getNodeCount(); j++)
		{
			const dotX39::Node* nodeNode = node->getNode(j);
			if (nodeNode->getName().compare("keywords") == 0)
			{
				for (int k = 0; k < nodeNode->getDataCount(); k++)
				{
					const dotX39::Data* keyword = nodeNode->getData(k);
					if (keyword->getType() != dotX39::DataTypes::STRING)
					{
						cout << string("the nodes '").append(node->getName()).append("' keywords node contains non-string entry '").append(keyword->getName()).append("'") << endl;
						exit(-1);
					}
					TEMPLATEKEYWORD tk;
					tk.name = keyword->getName();
					tk.text = ((dotX39::DataString*)keyword)->getDataAsString();
					r.keywords.push_back(tk);
				}
			}
			else
			{
				if (verbose)
					cout << string("the node '").append(node->getName()).append("' contains the unknown node '").append(nodeNode->getName()).append("'") << endl;
			}
		}
		replacementsFiles.push_back(r);
	}
}
void parseTemplates(vector<TEMPLATE>& templateFiles, const dotX39::Node* templates, string templateBasePath)
{
	//Create Template structures
	for (int i = 0; i < templates->getNodeCount(); i++)
	{
		const dotX39::Node* node = templates->getNode(i);
		TEMPLATE t;
		t.name = node->getName();
		//Read the templates arguments into the TEMPLATE structure
		for (int j = 0; j < node->getArgumentCount(); j++)
		{
			const dotX39::Data* arg = node->getArgument(j);
			if (arg->getName().compare("path") == 0)
			{
				if (arg->getType() != dotX39::DataTypes::STRING)
				{
					cout << string("the nodes '").append(node->getName()).append("' path argument should be STRING") << endl;
					exit(-1);
				}
				t.filePath = ((dotX39::DataString*)arg)->getDataAsString();
			}
			else if (arg->getName().compare("fileName") == 0)
			{
				if (arg->getType() != dotX39::DataTypes::STRING)
				{
					cout << string("the nodes '").append(node->getName()).append("' fileName argument should be STRING") << endl;
					exit(-1);
				}
				t.fileName = ((dotX39::DataString*)arg)->getDataAsString();
			}
			else if (arg->getName().compare("fileExtension") == 0)
			{
				if (arg->getType() != dotX39::DataTypes::STRING)
				{
					cout << string("the nodes '").append(node->getName()).append("' fileExtension argument should be STRING") << endl;
					exit(-1);
				}
				t.fileExtension = ((dotX39::DataString*)arg)->getDataAsString();
			}
			else
			{
				if (verbose)
					cout << string("the node '").append(node->getName()).append("' contains the unknown argument '").append(arg->getName()).append("'") << endl;
			}
		}
		//Load template file into RAM
		if (t.filePath.empty())
		{
			cout << string("the node '").append(node->getName()).append("' has no path argument provided") << endl;
			exit(-1);
		}
		ifstream stream = ifstream(string().append(templateBasePath).append(t.filePath).c_str());
		if (!stream.is_open() || !stream.good())
		{
			cout << string("Could not read file: ").append(string().append(templateBasePath).append(t.filePath)) << endl;
			exit(-1);
		}
		char s[256];
		while (!stream.eof())
		{
			memset(s, 0, sizeof(s));
			stream.read(s, 255);
			t.content.append(s);
		}
		stream.close();
		for (int j = 0; j < node->getDataCount(); j++)
		{
			const dotX39::Data* data = node->getData(j);
			if (data->getName().compare("ressources") == 0)
			{
				if (data->getType() != dotX39::DataTypes::ARRAY)
				{
					cout << string("the nodes '").append(node->getName()).append("' ressources argument should be an ARRAY of strings") << endl;
					exit(-1);
				}
				//Read the templates array and put it into the structure
				for (int k = 0; k < ((dotX39::DataArray*)data)->getDataCount(); k++)
				{
					const dotX39::Data* arrayData = ((dotX39::DataArray*)data)->getDataElement(k);
					if (arrayData->getType() != dotX39::DataTypes::STRING)
					{
						cout << string("the nodes '").append(node->getName()).append("' ressources data contains non-string entries") << endl;
						exit(-1);
					}
					t.usedRessources.push_back(((dotX39::DataString*)arrayData)->getDataAsString());
				}
			}
			else
			{
				if (verbose)
					cout << string("the node '").append(node->getName()).append("' contains the unknown data '").append(data->getName()).append("', ignored.") << endl;
			}
		}
		//Load the different keyword combinations into the TEMPLATE structure
		for (int j = 0; j < node->getNodeCount(); j++)
		{
			const dotX39::Node* keywordNode = node->getNode(j);
			if (keywordNode->getName().compare("templateKeywords") == 0)
			{
				for (int j = 0; j < keywordNode->getDataCount(); j++)
				{
					TEMPLATEKEYWORD tk;
					const dotX39::Data* data = keywordNode->getData(j);
					if (data->getType() != dotX39::DataTypes::STRING)
					{
						cout << string("the nodes '").append(node->getName()).append("' templateKeywords data has to be of the Type String, not valid for at least '").append(data->getName()).append("'") << endl;
						exit(-1);
					}
					tk.text = ((dotX39::DataString*)data)->getDataAsString();
					tk.name = data->getName();
					t.keywordsTemplate.push_back(tk);
				}
			}
			else if (keywordNode->getName().compare("replacementKeywords") == 0)
			{
				for (int j = 0; j < keywordNode->getDataCount(); j++)
				{
					REPLACEMENTKEYWORD rk;
					const dotX39::Data* data = keywordNode->getData(j);
					if (data->getType() != dotX39::DataTypes::ARRAY)
					{
						cout << string("the nodes '").append(node->getName()).append("' templateKeywords data has to be of the Type Array with two Strings inside, not valid for at least '").append(data->getName()).append("'") << endl;
						exit(-1);
					}
					dotX39::Data* dataArray_Entry1 = ((dotX39::DataArray*)data)->getDataElement(0);
					dotX39::Data* dataArray_Entry2 = ((dotX39::DataArray*)data)->getDataElement(1);
					if (dataArray_Entry1 == NULL || dataArray_Entry2 == NULL || dataArray_Entry1->getType() != dotX39::DataTypes::STRING || dataArray_Entry2->getType() != dotX39::DataTypes::STRING)
					{
						cout << string("the nodes '").append(node->getName()).append("' templateKeywords data has to be of the Type Array with two Strings inside, not valid for at least '").append(data->getName()).append("'") << endl;
						exit(-1);
					}
					rk.name = data->getName();
					rk.keywordAppeariance = ((dotX39::DataString*)dataArray_Entry1)->getDataAsString();
					rk.keywordReplacement = ((dotX39::DataString*)dataArray_Entry2)->getDataAsString();
					t.keywordsReplace.push_back(rk);
				}
			}
			else
			{
				if (verbose)
					cout << string("the node '").append(keywordNode->getName()).append("' is unknown for templates") << endl;
			}
		}
		templateFiles.push_back(t);
	}
}
void parseRessources(vector<RESSOURCE>& ressourceFiles, const dotX39::Node* ressources, string ressourceBasePath)
{
	//Create Template structures
	for (int i = 0; i < ressources->getNodeCount(); i++)
	{
		const dotX39::Node* node = ressources->getNode(i);
		RESSOURCE r;
		r.name = node->getName();
		//Read the templates arguments into the TEMPLATE structure
		for (int j = 0; j < node->getArgumentCount(); j++)
		{
			const dotX39::Data* arg = node->getArgument(j);
			if (arg->getName().compare("path") == 0)
			{
				if (arg->getType() != dotX39::DataTypes::STRING)
				{
					cout << string("the nodes '").append(node->getName()).append("' path argument should be STRING") << endl;
					exit(-1);
				}
				r.filePath = std::string(ressourceBasePath).append(((dotX39::DataString*)arg)->getDataAsString());
			}
			else if (arg->getName().compare("outname") == 0)
			{
				if (arg->getType() != dotX39::DataTypes::STRING)
				{
					cout << string("the nodes '").append(node->getName()).append("' outname argument should be STRING") << endl;
					exit(-1);
				}
				r.outname = ((dotX39::DataString*)arg)->getDataAsString();
			}
			else
			{
				if (verbose)
					cout << string("the node '").append(node->getName()).append("' contains the unknown argument '").append(arg->getName()).append("', ignored.") << endl;
			}
		}
		ressourceFiles.push_back(r);
	}
}
int createDirectory(string path)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	wstring wStringPath = converter.from_bytes(path);
	auto result = CreateDirectory(wStringPath.c_str(), NULL);
	if (result == ERROR_PATH_NOT_FOUND)
	{
		createDirectory(path.substr(0, path.rfind('\\')));
		return createDirectory(path);
	}
	return result;
}
int createDirectory(wstring path)
{
	auto result = CreateDirectory(path.c_str(), NULL);
	if (result == ERROR_PATH_NOT_FOUND)
	{
		createDirectory(path.substr(0, path.rfind('\\')));
		return createDirectory(path);
	}
	return result;
}
void writeRessource(vector<RESSOURCE>& ressources, string ressourceName, string outfolder)
{
	RESSOURCE* res = NULL;
	for (int j = 0; j < ressources.size(); j++)
	{
		if (ressources[j].name.compare(ressourceName) == 0)
		{
			res = &ressources[j];
			break;
		}
	}
	if (res == NULL)
	{
		cout << string("The RESSOURCE ").append(ressourceName).append(" is not existing") << endl;
		exit(-1);
	}
	CopyFileA(res->filePath.c_str(), std::string(outfolder).append(res->outname).c_str(), false);
}
void writeFile(REPLACEMENT& r, vector<TEMPLATE>& templateFiles, vector<RESSOURCE>& ressources)
{
	if (verbose)
		cout << "Creating directory " << r.folderPath << "' if it not exists" << endl;
	createDirectory(r.folderPath);
	for (int i = 0; i < r.usedRessources.size(); i++)
		writeRessource(ressources, r.usedRessources[i], r.folderPath);
	for (int i = 0; i < r.usedTemplates.size(); i++)
	{
		TEMPLATE* t = NULL;
		for (int j = 0; j < templateFiles.size(); j++)
		{
			if (templateFiles[j].name.compare(r.usedTemplates[i]) == 0)
			{
				t = &templateFiles[j];
				break;
			}
		}
		if (t == NULL)
		{
			cout << string("The TEMPLATE ").append(r.usedTemplates[i]).append(" of replacement ").append(r.name).append(" is not existing") << endl;
			exit(-1);
		}
		for (int i = 0; i < t->usedRessources.size(); i++)
			writeRessource(ressources, t->usedRessources[i], r.folderPath);
		string fileName = string(t->fileName).append(".").append(t->fileExtension);
		REPLACEMENT rCopy = REPLACEMENT(r);
		//Replace TemplateKeywords in the replacement
		for (int j = 0; j < t->keywordsReplace.size(); j++)
		{
			REPLACEMENTKEYWORD* rk = &t->keywordsReplace[j];
			bool flag = false;
			for (int k = 0; k < rCopy.keywords.size(); k++)
			{
				TEMPLATEKEYWORD* tk = &rCopy.keywords[k];
				int findResult = tk->text.find(rk->keywordAppeariance);
				if (findResult != -1)
				{
					//Insert replacement
					tk->text = string(tk->text.substr(0, findResult)).append(rk->keywordReplacement).append(tk->text.substr(findResult + rk->keywordAppeariance.length()));
					flag = true;
				}
			}
			if (!flag && verbose)
			{
				cout << "The ReplacementKeyword '" << rk->name << "' is unused for " << r.name << endl;
			}
		}
		//Make sure ALL keywords the template requires are set and exit if not all are set proper
		for (int j = 0; j < t->keywordsTemplate.size(); j++)
		{
			TEMPLATEKEYWORD* tk = &t->keywordsTemplate[j];
			bool flag = false;
			for (int k = 0; k < rCopy.keywords.size(); k++)
			{
				TEMPLATEKEYWORD* tk = &rCopy.keywords[k];
				if (tk->name.compare(tk->name) == 0)
				{
					flag = true;
					break;
				}
			}
			if (!flag)
			{
				cout << "The TemplateKeyword '" << tk->name << "' was not set for " << r.name << endl;
				exit(-1);
			}
		}
		if (verbose)
			cout << "Creating file '" << fileName << "' from template '" << t->name << "'" << endl;
		ofstream stream = ofstream(string(rCopy.folderPath).append(t->fileName).append(".").append(t->fileExtension));
		string content = string(t->content);
		for (int j = 0; j < t->keywordsTemplate.size(); j++)
		{
			TEMPLATEKEYWORD* templateKeyword = &t->keywordsTemplate[j];
			for (int k = 0; k < rCopy.keywords.size(); k++)
			{
				TEMPLATEKEYWORD* replacementKeyword = &rCopy.keywords[k];
				if (replacementKeyword->name.compare(templateKeyword->name) == 0)
				{
					int findResult = content.find(templateKeyword->text);
					if (findResult == -1)
					{
						if (verbose)
							cout << "Template " << t->name << " has the unused keyword '" << templateKeyword->name << "'" << endl;
					}
					else
					{
						do {
							//Insert replacement
							content = string(content.substr(0, findResult)).append(replacementKeyword->text).append(content.substr(findResult + templateKeyword->text.length()));
						} while ((findResult = content.find(templateKeyword->text)) != -1);
					}
					break;
				}
			}
		}
		stream.write(content.c_str(), content.size());
		stream.close();
	}
}
int main(int argc, char* argv[])
{
	basePath = string();
	projectFileName = string();
	verbose = false;
	const dotX39::Node* templates = NULL;
	string templateBasePath = "";
	const dotX39::Node* replacements = NULL;
	string replacementsBasePath = "";
	const dotX39::Node* ressources = NULL;
	string ressourcesBasePath = "";
	vector<TEMPLATE> templateFiles = vector<TEMPLATE>();
	vector<RESSOURCE> ressourceFiles = vector<RESSOURCE>();
	vector<REPLACEMENT> replacementsFiles = vector<REPLACEMENT>();
	CommandHandler cmdHandler = CommandHandler('-');
	string argument;

	cmdHandler.registerCommand(COMMAND(cmdHelp, "?", "Outputs an overview of available commands", false));
	cmdHandler.registerCommand(COMMAND(cmdPath, "path", "Input argument that has to point to a project.x39 file", true));
	cmdHandler.registerCommand(COMMAND(cmdVerbose, "verbose", "Enables extended output", false));
	cmdHandler.registerCommand(COMMAND(cmdAbout, "about", "Prints informations about the author and 'license'", false));
	cmdHandler.registerCommand(COMMAND(cmdVersion, "version", "Prints current version number", false));
	if (argc == 1)
	{
		cout << "No Arguments provided, use -? for a basic help page" << endl;
		exit(-1);
	}
	argument = argv[1];
	for (int i = 2; i < argc; i++)
	{
		if (argv[i][0] != '-')
			if (strchr(argv[i], ' ') == NULL)
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
	if (basePath.empty() && projectFileName.empty())
	{
		if (verbose)
			cout << "No path given, please provide a path argument" << endl;
		exit(-1);
	}
	if (verbose)
		cout << "Reading project document file" << endl;
	dotX39::Node* documentRoot = new dotX39::Node("root");
	try
	{
		dotX39::DocumentReader::readDocument(string().append(basePath).append(projectFileName), documentRoot);
	}
	catch (exception e)
	{
		cout << "Ran into problems while reading the project file :(" << endl;
		cout << e.what() << endl;
		exit(-1);
	}
	//Gather important informations about the 2 base nodes
	for (int i = 0; i < documentRoot->getNodeCount(); i++)
	{
		const dotX39::Node* node = documentRoot->getNode(i);
		if (node->getName().compare("templates") == 0)
		{
			templates = node;
			for (int j = 0; j < node->getArgumentCount(); j++)
			{
				const dotX39::Data* arg = node->getArgument(j);
				if (arg->getName().compare("basePath") == 0)
				{
					if (arg->getType() != dotX39::DataTypes::STRING)
					{
						cout << string("the nodes '").append(node->getName()).append("' basePath argument should be STRING") << endl;
						exit(-1);
					}
					templateBasePath = string().append(basePath).append(((dotX39::DataString*)arg)->getDataAsString());
				}
				else
				{
					if (verbose)
						cout << string("the node '").append(node->getName()).append("' contains the unknown argument '").append(arg->getName()).append("', ignored.") << endl;
				}
			}
		}
		else if (node->getName().compare("ressources") == 0)
		{
			ressources = node;
			for (int j = 0; j < node->getArgumentCount(); j++)
			{
				const dotX39::Data* arg = node->getArgument(j);
				if (arg->getName().compare("basePath") == 0)
				{
					if (arg->getType() != dotX39::DataTypes::STRING)
					{
						exit(-1);
					}
					ressourcesBasePath = string().append(basePath).append(((dotX39::DataString*)arg)->getDataAsString());
				}
				else
				{
					if (verbose)
						cout << string("the node '").append(node->getName()).append("' contains the unknown argument '").append(arg->getName()).append("', ignored.") << endl;
				}
			}
		}
		else if (node->getName().compare("replacements") == 0)
		{
			replacements = node;
			for (int j = 0; j < node->getArgumentCount(); j++)
			{
				const dotX39::Data* arg = node->getArgument(j);
				if (arg->getName().compare("basePath") == 0)
				{
					if (arg->getType() != dotX39::DataTypes::STRING)
					{
						exit(-1);
					}
					replacementsBasePath = string().append(basePath).append(((dotX39::DataString*)arg)->getDataAsString());
				}
				else
				{
					if (verbose)
						cout << string("the node '").append(node->getName()).append("' contains the unknown argument '").append(arg->getName()).append("', ignored.") << endl;
				}
			}
		}
		else
		{
			if (verbose)
				cout << string("the node '").append(node->getName()).append("' is Unknown as base node") << endl;
		}
	}

	//Create Ressources Structures
	if (ressources != NULL)
	{
		parseRessources(ressourceFiles, ressources, ressourcesBasePath);
	}
	//Create Replacement Structures
	if (templates == NULL)
	{
		cout << "Was unable to find the 'templates' node" << endl;
		exit(-1);
	}
	parseTemplates(templateFiles, templates, templateBasePath);
	//Create Replacement Structures
	if (replacements == NULL)
	{
		cout << "Was unable to find the 'replacements' node" << endl;
		exit(-1);
	}
	parseReplacements(replacementsFiles, replacements, replacementsBasePath);
	delete documentRoot;
	for (int i = 0; i < replacementsFiles.size(); i++)
	{
		if (verbose)
			cout << endl << string(20, '-') << endl << "Creating files for " << replacementsFiles[i].name << endl;
		else
			cout << "Creating files for " << replacementsFiles[i].name << endl;
		writeFile(replacementsFiles[i], templateFiles, ressourceFiles);
		if (verbose)
			cout << string(20, '-') << endl;
	}
	cout << "---DONE---" << endl;
	exit(1);
}