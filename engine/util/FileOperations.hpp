#pragma once
#include "CommonTypes.hpp"

#include <string>
#include <vector>

namespace fileop
{
    std::vector<u8> read_binary_file(const std::string& file_name); // reads all bytes from binaries and returns byte array
    void write_binary_file(void* data, size_t size, const char* file_name);
    std::vector<std::string> read_file_to_vector(const std::string& filename);
    std::string file_to_string(const char* file_path);
    void write_to_file(const char* file_path, const std::string& src);
    void overwrite_file(const char* file_path, const std::string& src);
}
