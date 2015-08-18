#pragma once
#include "../global.hpp"
#include "Ressource.hpp"
#include "TemplateKeyword.hpp"
#include "ReplacementKeyword.hpp"

#include "../dotX39/Node.h"
#include "../dotX39/Data.h"
#include "../dotX39/DataString.h"
#include "../dotX39/DataArray.h"

#include <string>
#include <vector>
#include <fstream>

namespace XTR
{
	class Template
	{
	private:
		std::string qualifier;
		std::string filePath;
		std::string outputFileName;
		std::string outputFileExtension;
		std::string fileData;
		std::vector<const Ressource&> ressources;
		struct Keywords {
			std::vector<TemplateKeyword> templates;
			std::vector<ReplacementKeyword> replacements;
		} keywords;

		Template() {}
	public:

		static Template& parse(const dotX39::Node& node, const std::vector<Ressource> ressourceList)
		{
			auto& tmp = Template();
			tmp.qualifier = node.getName();

#pragma region Argument Parsing
			for (int i = 0; i < node.getArgumentCount(); i++)
			{
				const dotX39::Data* arg = node.getArgument(i);
				if (arg->getName().compare("path") == 0)
				{
					if (arg->getType() != dotX39::DataTypes::STRING)
					{
						std::cerr << std::string("the nodes '").append(tmp.qualifier).append("' 'path' argument should be STRING") << std::endl;
						throw Exceptions::InvalidTypeStructureException();
					}
					tmp.filePath = std::string(Globals::getInstance().basePath.ressources).append(static_cast<const dotX39::DataString*>(arg)->getDataAsString());
				}
				else if (arg->getName().compare("fileName") == 0)
				{
					if (arg->getType() != dotX39::DataTypes::STRING)
					{
						std::cerr << std::string("the nodes '").append(tmp.qualifier).append("' 'fileName' argument should be STRING") << std::endl;
						throw Exceptions::InvalidTypeStructureException();
					}
					tmp.outputFileName = std::string(Globals::getInstance().basePath.ressources).append(static_cast<const dotX39::DataString*>(arg)->getDataAsString());
				}
				else if (arg->getName().compare("fileExtension") == 0)
				{
					if (arg->getType() != dotX39::DataTypes::STRING)
					{
						std::cerr << std::string("the nodes '").append(tmp.qualifier).append("' 'fileExtension' argument should be STRING") << std::endl;
						throw Exceptions::InvalidTypeStructureException();
					}
					tmp.outputFileExtension = std::string(Globals::getInstance().basePath.ressources).append(static_cast<const dotX39::DataString*>(arg)->getDataAsString());
				}
				else if (arg->getName().compare("ressources") == 0)
				{
					if (arg->getType() != dotX39::DataTypes::ARRAY)
					{
						std::cerr << std::string("the nodes '").append(tmp.qualifier).append("' 'ressources' argument should be ARRAY") << std::endl;
						throw Exceptions::InvalidTypeStructureException();
					}
					auto tmpDataArray = static_cast<const dotX39::DataArray*>(arg);
					//We want to validate ALL ressources here so lets create an error counter instead of throwing an exception instantly
					size_t errCount = 0;
					for (int j = 0; j < tmpDataArray->getDataCount(); j++)
					{
						auto data = tmpDataArray->getDataElement(j);
						if (arg->getType() != dotX39::DataTypes::STRING)
						{
							std::cerr << std::string("the nodes '").append(tmp.qualifier).append("' 'ressource' argument should be ARRAY with just STRINGs") << std::endl;
							throw Exceptions::InvalidTypeStructureException();
						}
						//Find the ressource in available ressources and push it to the templates ressource list
						bool flag = false;
						auto ressourceName = static_cast<const dotX39::DataString*>(data)->getDataAsString();
						for (auto& it : ressourceList)
						{
							if (it.getQualifier().compare(ressourceName) == 0)
							{
								tmp.ressources.push_back(it);
								flag = true;
								break;
							}
						}
						if (!flag)
						{
							errCount++;
							std::cerr << "Ressource '" << ressourceName << "' is not existing (template: '" << tmp.qualifier << "')" << std::endl;
						}
					}
					//if we did found errors, throw the invalid_argument exception
					if (errCount > 0)
						throw std::invalid_argument("Some Ressources are not existing for a Template");
				}
				else
				{
					if (Globals::getInstance().verbosity)
						std::cout << std::string("the node '").append(tmp.qualifier).append("' contains the unknown argument '").append(arg->getName()).append("', ignored.") << std::endl;
				}
			}
#pragma endregion
#pragma region Node Parsing
			for (int i = 0; i < node.getNodeCount(); i++)
			{
				auto subnode = node.getNode(i);
				if (subnode->getName().compare("templateKeywords") == 0)
				{
					for (int j = 0; j < subnode->getDataCount(); j++)
					{
						auto data = subnode->getData(j);
						if (data->getType() != dotX39::DataTypes::STRING)
						{
							std::cerr << std::string("the nodes '").append(tmp.qualifier).append("' subnodes '").append(subnode->getName()).append("' data datatypes should be STRING") << std::endl;
							throw Exceptions::InvalidTypeStructureException();
						}
						tmp.keywords.templates.push_back(TemplateKeyword::parse(*static_cast<const dotX39::DataString*>(data)));
					}
				}
				else if (subnode->getName().compare("replacementKeywords") == 0)
				{
					for (int j = 0; j < subnode->getDataCount(); j++)
					{
						auto data = subnode->getData(j);
						if (data->getType() != dotX39::DataTypes::ARRAY)
						{
							std::cerr << std::string("the nodes '").append(tmp.qualifier).append("' subnodes '").append(subnode->getName()).append("' data datatypes should be ARRAY") << std::endl;
							throw Exceptions::InvalidTypeStructureException();
						}
						tmp.keywords.replacements.push_back(ReplacementKeyword::parse(*static_cast<const dotX39::DataArray*>(data)));
					}
				}
				else
				{
					if (Globals::getInstance().verbosity)
						std::cout << std::string("the node '").append(tmp.qualifier).append("' contains the unknown node '").append(subnode->getName()).append("', ignored.") << std::endl;
				}
			}
#pragma endregion

			if (tmp.filePath.empty())
			{
				std::cerr << std::string("the node '").append(tmp.getQualifier()).append("' has no path argument provided") << std::endl;
				exit(-1);
			}
			std::ifstream stream = std::ifstream(std::string(Globals::getInstance().basePath.templates).append(tmp.filePath).c_str());
			if (!stream.is_open() || !stream.good())
			{
				auto errMsg = std::string("Could not read file: ").append(std::string(Globals::getInstance().basePath.templates).append(tmp.filePath));
				std::cerr << errMsg << std::endl;
				throw std::runtime_error(errMsg);
			}
			char s[256];
			while (!stream.eof())
			{
				memset(s, 0, sizeof(s));
				stream.read(s, 255);
				tmp.fileData.append(s);
			}
			stream.close();
			return tmp;
		}
		std::string patchData(const std::vector<const TemplateKeyword> keywords) const
		{
			throw std::exception("not implemented");
		}
		std::string validateKeywords(const std::vector<const TemplateKeyword> keywords) const
		{
			throw std::exception("not implemented");
		}
		const std::string& getQualifier() const { return qualifier; }
		const std::string& getFilePath() const { return filePath; }
		const std::string& getOutputFileName() const { return outputFileName; }
		const std::string& getFileData() const { return fileData; }
		const std::string& getOutputFileExtension() const { return outputFileExtension; }
		const std::vector<const Ressource&>& getRessources() const { return ressources; }
		const Keywords& getKeywords() const { return keywords; }
	};
}