#include "pch.h"
#include "Primitives.h"

#include "Buffer.h"

#include <cstring>

std::string primitve_type_to_str(PrimitiveTypes pt)
{
	switch (pt)
	{
	case PrimitiveTypes::None:
		return "none";
	case PrimitiveTypes::Cube:
		return "cube";
	case PrimitiveTypes::Quad:
		return "quad";
	default:
		break;
	}
}

PrimitiveTypes str_to_primitive_type(const char* name)
{
	if (!strcmp(name, "cube"))
	{
		return PrimitiveTypes::Cube;
	}
	else if (!strcmp(name, "quad"))
	{
		return PrimitiveTypes::Quad;
	}

	return PrimitiveTypes::None;
}