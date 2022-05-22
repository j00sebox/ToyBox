#pragma once

#include <string>

std::string file_to_string(const char* file_path);
void write_to_file(const char* file_path, const std::string& src);
void overwrite_file(const char* file_path, const std::string& src);