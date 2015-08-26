#pragma once
#include "Ressource.hpp"
#include "Template.hpp"
#include "TemplateKeyword.hpp"

#include <string>
#include <vector>

namespace XTR
{
	class Replacement
	{
	private:
		std::string qualifier;
		std::string outputFolder;
		std::vector<const Template> templates;
		std::vector<const Ressource> ressources;
		std::vector<TemplateKeyword> templateKeywords;

		Replacement() {}
	public:
		static Replacement parse(const dotX39::Node& node, const std::vector<XTR::Template>& templateList, const std::vector<XTR::Ressource>& ressourceList)
		{
			auto tmp = Replacement();
			tmp.qualifier = node.getName();
			//Read the arguments into the REPLACEMENT structure
			for (int j = 0; j < node.getArgumentCount(); j++)
			{
				const dotX39::Data* arg = node.getArgument(j);
				if (arg->getName().compare("path") == 0)
				{
					if (arg->getType() != dotX39::DataTypes::STRING)
					{
						std::cout << std::string("the nodes '").append(node.getName()).append("' path argument should be STRING") << std::endl;
						exit(-1);
					}
					tmp.outputFolder = std::string().append(Globals::getInstance().basePath.replacement).append(((dotX39::DataString*)arg)->getDataAsString());
				}
				else
				{
					if (Globals::getInstance().verbosity)
						std::cout << std::string("the node '").append(node.getName()).append("' contains the unknown argument '").append(arg->getName()).append("'") << std::endl;
				}
			}
			for (int j = 0; j < node.getDataCount(); j++)
			{
				const dotX39::Data* data = node.getData(j);
				if (data->getName().compare("templates") == 0)
				{
					if (data->getType() != dotX39::DataTypes::ARRAY)
					{
						std::cout << std::string("the nodes '").append(node.getName()).append("' templates argument should be an ARRAY of strings") << std::endl;
						exit(-1);
					}
					//Read the templates array and put it into the structure
					for (int k = 0; k < ((dotX39::DataArray*)data)->getDataCount(); k++)
					{
						const dotX39::Data* arrayData = ((dotX39::DataArray*)data)->getDataElement(k);
						if (arrayData->getType() != dotX39::DataTypes::STRING)
						{
							std::cout << std::string("the nodes '").append(node.getName()).append("' templates data contains non-string entries") << std::endl;
							exit(-1);
						}
						auto s = ((dotX39::DataString*)arrayData)->getDataAsString();
						for (auto& it : templateList)
						{
							if (it.getQualifier().compare(s) == 0)
							{
								tmp.templates.push_back(it);
								break;
							}
						}
					}
				}
				else if (data->getName().compare("ressources") == 0)
				{
					if (data->getType() != dotX39::DataTypes::ARRAY)
					{
						std::cout << std::string("the nodes '").append(node.getName()).append("' ressources argument should be an ARRAY of strings") << std::endl;
						exit(-1);
					}
					//Read the templates array and put it into the structure
					for (int k = 0; k < ((dotX39::DataArray*)data)->getDataCount(); k++)
					{
						const dotX39::Data* arrayData = ((dotX39::DataArray*)data)->getDataElement(k);
						if (arrayData->getType() != dotX39::DataTypes::STRING)
						{
							std::cout << std::string("the nodes '").append(node.getName()).append("' ressources data contains non-string entries") << std::endl;
							exit(-1);
						}
						auto s = ((dotX39::DataString*)arrayData)->getDataAsString();
						for (auto& it : ressourceList)
						{
							if (it.getQualifier().compare(s) == 0)
							{
								tmp.ressources.push_back(it);
								break;
							}
						}
					}
				}
				else
				{
					if (Globals::getInstance().verbosity)
						std::cout << std::string("the node '").append(node.getName()).append("' contains the unknown data '").append(data->getName()).append("', ignored.") << std::endl;
				}
			}
			for (int j = 0; j < node.getNodeCount(); j++)
			{
				const dotX39::Node* subnode = node.getNode(j);
				if (subnode->getName().compare("keywords") == 0)
				{
					for (int k = 0; k < subnode->getDataCount(); k++)
						tmp.templateKeywords.push_back(TemplateKeyword::parse(*subnode->getData(k)));
				}
				else
				{
					if (Globals::getInstance().verbosity)
						std::cout << std::string("the node '").append(node.getName()).append("' contains the unknown node '").append(subnode->getName()).append("'") << std::endl;
				}
			}


			return tmp;
		}
		const std::string& getQualifier() const { return qualifier; }
		const std::string& getOutputFolder() const { return outputFolder; }
		const std::vector<const Template>& getTemplates() const { return templates; }
		const std::vector<const Ressource>& getRessources() const { return ressources; }
		const std::vector<TemplateKeyword>& getTemplateKeywords() const { return templateKeywords; }
	};
}