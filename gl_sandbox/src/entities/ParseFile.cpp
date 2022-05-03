#include "pch.h"
#include "ParseFile.h"

std::string file_to_string(const char* file_path)
{
	std::string line;
	std::ifstream stream(file_path);

	std::stringstream ss;

	while (getline(stream, line))
	{
		ss << line << "\n";
	}

	stream.close();

	return ss.str();
}