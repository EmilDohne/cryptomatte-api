#pragma once

#include <climits>

#define NAMESPACE_CRYPTOMATTE_API cmatte

// Check that float and double are 32 and 64 bit wide respectively 
static_assert(sizeof(float) == 4 && CHAR_BIT == 8 && std::numeric_limits<float>::is_iec559, "float type is not 32 bit wide, this is not currently supported");

// Alias these types for consistency throughout the code
typedef float float32_t;