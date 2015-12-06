#pragma once

#include <type_traits>

#include "exception.hpp"

#include <GL/glew.h>

namespace gl {
enum Type {
	INT,
	FLOAT
};
/*
template <typename T>
class get_type {
	static const typename std::enable_if<std::is_floating_point<T>::value && sizeof(T) == sizeof(GLfloat), Type>::type value = FLOAT;
	static const typename std::enable_if<std::is_integral<T>::value && sizeof(T) == sizeof(GLint), Type>::type value = INT;
};
*/
template <typename T>
struct get_type {
	
};
template <>
struct get_type<float> {
	static const Type value = FLOAT;
};
template <>
struct get_type<int> {
	static const Type value = INT;
};
}
