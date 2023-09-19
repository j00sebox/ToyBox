#pragma once
#include "Types.hpp"

#include <string>
#include <vector>

namespace fileop
{
    // reads all bytes from binaries and returns byte array
    std::vector<u8> read_binary_file(const std::string& file_name);

    void write_binary_file(void* data, size_t size, const char* file_name);

    std::vector<std::string> read_file_to_vector(const std::string& filename);

}
