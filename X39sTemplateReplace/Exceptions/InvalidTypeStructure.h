#pragma once
#include <exception>
namespace Exceptions
{
	class InvalidTypeStructureException : public std::runtime_error
	{
	public:
		InvalidTypeStructureException() : std::runtime_error("Invalid input, project file contains invalid typestructure") {}
	};
}