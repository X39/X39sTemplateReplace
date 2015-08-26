#pragma once
#include "../dotX39/Data.h"
#include "../dotX39/DataString.h"
#include "../Exceptions/InvalidTypeStructure.h"
#include <string>
namespace XTR
{
	class TemplateKeyword
	{
	private:
		std::string qualifier;
		std::string text;
		TemplateKeyword() {}
	public:
		TemplateKeyword(std::string qualifier, std::string text) : qualifier(qualifier), text(text) {}
		static TemplateKeyword parse(const dotX39::Data& data)
		{
			auto tmp = TemplateKeyword();
			tmp.qualifier = data.getName();
			if (data.getType() != dotX39::DataTypes::STRING)
				throw Exceptions::InvalidTypeStructureException();
			tmp.text = static_cast<const dotX39::DataString>(static_cast<const dotX39::DataString&>(data)).getDataAsString();
			return tmp;
		}
		const std::string& getQualifier() const { return qualifier; }
		const std::string& getText() const { return text; }
	};
}