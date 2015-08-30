#pragma once
#include <string>

class Globals
{
public:
	bool verbosity;
	bool exitAfterArgReading;
	struct
	{
		std::string path;
		std::string name;
		void clear()
		{
			path.clear();
			name.clear();
		}
	} projectFile;

	struct
	{
		std::string ressources;
		std::string templates;
		std::string replacement;
	} basePath;

	const std::string version = std::string("2.0.1");
	Globals() : verbosity(false), exitAfterArgReading(true) {}

	static inline Globals& getInstance()
	{
		static Globals globals;
		return globals;
	}
};