#include "pch.h"
#include "FileOperations.hpp"

namespace fileop
{
    std::vector<u8> read_binary_file(const std::string& file_name)
    {
        std::ifstream file(file_name, std::ios::ate | std::ios::binary);

        if(!file.is_open())
        {
            throw std::runtime_error("failed to open file!");
        }

        size_t file_size = (size_t)file.tellg();
        std::vector<u8> buffer(file_size);

        // go back to beginning of file and read it
        file.seekg(0);
        file.read(buffer.data(), file_size);

        file.close();

        return buffer;
    }

    void write_binary_file(void* data, size_t size, const char* file_name)
    {
        std::ofstream  file(file_name, std::ios::binary);

        if(!file.is_open())
        {
            throw std::runtime_error("failed to open file!");
        }

        file.write((char*)data, size);

        file.close();
    }

    std::vector<std::string> read_file_to_vector(const std::string& filename)
    {
        std::ifstream source;
        source.open(filename);
        std::vector<std::string> lines;
        std::string line;
        while (std::getline(source, line))
        {
            lines.push_back(line);
        }
        return lines;
    }
}