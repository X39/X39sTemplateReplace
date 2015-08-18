#pragma once
#include "../Exceptions/InvalidTypeStructure.h"
#include "../dotX39/Node.h"
#include "../dotX39/Data.h"
#include "../dotX39/DataString.h"
#include "../global.hpp"
#include <string>
#include <iostream>
namespace XTR
{
	class Ressource
	{
	private:
		std::string qualifier;
		std::string filePath;
		std::string outputFileName;
		Ressource() {}
	public:
		static Ressource& parse(const dotX39::Node& node)
		{
			auto& tmp = Ressource();
			tmp.qualifier = node.getName();
			for (int i = 0; i < node.getArgumentCount(); i++)
			{
				const dotX39::Data* arg = node.getArgument(i);
				if (arg->getName().compare("path") == 0)
				{
					if (arg->getType() != dotX39::DataTypes::STRING)
					{
						std::cerr << std::string("the nodes '").append(tmp.qualifier).append("' path argument should be STRING") << std::endl;
						throw Exceptions::InvalidTypeStructureException();
					}
					tmp.filePath = std::string(Globals::getInstance().basePath.tmps).append(static_cast<const dotX39::DataString*>(arg)->getDataAsString());
				}
				else if (arg->getName().compare("outname") == 0)
				{
					if (arg->getType() != dotX39::DataTypes::STRING)
					{
						std::cerr << std::string("the nodes '").append(tmp.qualifier).append("' outname argument should be STRING") << std::endl;
						throw Exceptions::InvalidTypeStructureException();
					}
					tmp.outputFileName = static_cast<const dotX39::DataString*>(arg)->getDataAsString();
				}
				else
				{
					if (Globals::getInstance().verbosity)
						std::cout << std::string("the node '").append(tmp.qualifier).append("' contains the unknown argument '").append(arg->getName()).append("', ignored.") << std::endl;
				}
			}
			return tmp;
		}
		const std::string& getQualifier() const { return qualifier; }
		const std::string& getFilePath() const { return filePath; }
		const std::string& getOutputFileName() const { return outputFileName; }

	};
}