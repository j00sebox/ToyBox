#include "pch.h"
#include "FileOperations.h"

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

void write_to_file(const char* file_path, const std::string& src)
{
	std::ofstream out;

	out.open(file_path, std::ios_base::app);
	out << src;
}

void overwrite_file(const char* file_path, const std::string& src)
{
	std::ofstream out;

	out.open(file_path, std::ios_base::trunc);
	out << src;
}
