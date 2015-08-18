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
		static TemplateKeyword& parse(const dotX39::DataString& data)
		{
			auto& tmp = TemplateKeyword();
			tmp.qualifier = data.getName();
			if (data.getType() != dotX39::DataTypes::STRING)
				throw Exceptions::InvalidTypeStructureException();
			tmp.text = static_cast<const dotX39::DataString>(data).getDataAsString();
			return tmp;
		}
		const std::string& getQualifier() { return qualifier; }
		const std::string& getText() { return text; }
	};
}