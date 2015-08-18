#pragma once
#include "../dotX39/Data.h"
#include "../dotX39/DataString.h"
#include "../dotX39/DataArray.h"
#include "../Exceptions/InvalidTypeStructure.h"
#include <string>
namespace XTR
{
	class ReplacementKeyword
	{
	private:
		std::string qualifier;
		std::string appeariance;
		std::string replacement;
		ReplacementKeyword() {}
	public:
		static ReplacementKeyword& parse(const dotX39::DataArray& data)
		{
			auto& tmp = ReplacementKeyword();
			tmp.qualifier = data.getName();
			if (data.getDataCount() != 2 || data.getDataElement(0)->getType() != dotX39::DataTypes::STRING || data.getDataElement(1)->getType() != dotX39::DataTypes::STRING)
				throw Exceptions::InvalidTypeStructureException();
			tmp.appeariance = static_cast<const dotX39::DataString*>(data.getDataElement(0))->getDataAsString();
			tmp.replacement = static_cast<const dotX39::DataString*>(data.getDataElement(1))->getDataAsString();
			return tmp;
		}
		const std::string& getQualifier() { return qualifier; }
		const std::string& getAppeariance() { return appeariance; }
		const std::string& getReplacement() { return replacement; }
	};
}