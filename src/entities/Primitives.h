#pragma once

#include "VertexArray.h"

// TODO: Add more
enum class PrimitiveTypes
{
	None = 0,
	Cube,
	Quad
};

std::string primitve_type_to_str(PrimitiveTypes pt);
PrimitiveTypes str_to_primitive_type(const char* name);

struct Cube
{
    std::vector<float> vertices =
    {
            -1.0f, -1.0f,  1.0f, 0.f, 0.f, -1.f, 0.f, -1.f,		//        7--------6
            1.0f, -1.0f,  1.0f, 0.f, 0.f, -1.f, 0.f, -1.f,		//       /|       /|
            1.0f, -1.0f, -1.0f, 0.f, 0.f,  1.f, 0.f,  1.f,		//      4--------5 |
            -1.0f, -1.0f, -1.0f, 0.f, 0.f,  1.f, 0.f,  1.f,		//      | |      | |
            -1.0f,  1.0f,  1.0f, 0.f, 0.f, -1.f, 0.f, -1.f,		//      | 3------|-2
            1.0f,  1.0f,  1.0f, 0.f, 0.f, -1.f, 0.f, -1.f,		//      |/       |/
            1.0f,  1.0f, -1.0f, 0.f, 0.f,  1.f, 0.f,  1.f,		//      0--------1
            -1.0f,  1.0f, -1.0f, 0.f, 0.f,  1.f, 0.f,  1.f
    };

    std::vector<unsigned int> indices =
    {
            // right
            1, 2, 6,
            6, 5, 1,

            // left
            0, 4, 7,
            7, 3, 0,

            // top
            4, 5, 6,
            6, 7, 4,

            // bottom
            0, 3, 2,
            2, 1, 0,

            // back
            0, 1, 5,
            5, 4, 0,

            // front
            3, 7, 6,
            6, 2, 3
    };
};

struct Quad
{
    std::vector<float> vertices = {
            -0.5f, -0.5f, -0.5f, 0.f, 0.f, 1.f, 0.f, 0.f,
            -0.5f,	0.5f, -0.5f, 0.f, 0.f, 1.f, 0.f, 1.f,
            0.5f,  0.5f, -0.5f, 0.f, 0.f, 1.f, 1.f, 1.f,
            0.5f, -0.5f, -0.5f, 0.f, 0.f, 1.f, 1.f, 0.f
    };

    std::vector<unsigned int> indices = {
            0, 2, 1,
            0, 3, 2
    };
};