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
		std::vector<Template> templates;
		std::vector<Ressource> ressources;
		std::vector<TemplateKeyword> templateKeywords;

		Replacement() {}
	public:
		static Replacement parse(const dotX39::Node& node, const std::vector<XTR::Template>& templateList, const std::vector<XTR::Ressource>& ressourceList)
		{
			auto& tmp = Replacement();
			tmp.qualifier = node.getName();
			throw std::exception("not implemented");
			return tmp;
		}
		const std::string& getQualifier() { return qualifier; }
		const std::string& getOutputFolder() { return outputFolder; }
		const std::vector<Template>& getTemplates() { return templates; }
		const std::vector<Ressource>& getRessources() { return ressources; }
		const std::vector<TemplateKeyword>& getTemplateKeywords() { return templateKeywords; }
	};
}