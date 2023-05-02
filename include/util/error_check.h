#pragma once

#include "Log.h"
#include "glad/glad.h"

inline constexpr char const* GetErrorString(
	GLenum err) noexcept
{
	switch (err) {
		case GL_NO_ERROR:                      return "GL_NO_ERROR";
		case GL_INVALID_ENUM:                  return "GL_INVALID_ENUM";
		case GL_INVALID_VALUE:                 return "GL_INVALID_VALUE";
		case GL_INVALID_OPERATION:             return "GL_INVALID_OPERATION";
		case GL_STACK_OVERFLOW:                return "GL_STACK_OVERFLOW";
		case GL_STACK_UNDERFLOW:               return "GL_STACK_UNDERFLOW";
		case GL_OUT_OF_MEMORY:                 return "GL_OUT_OF_MEMORY";
		case GL_TABLE_TOO_LARGE:               return "GL_TABLE_TOO_LARGE";
		case GL_INVALID_FRAMEBUFFER_OPERATION: return "GL_INVALID_FRAMEBUFFER_OPERATION";
		default:                               return "unknown error";
	}
}

inline bool MessageCallback(
	const char* stmt,
	const char* fname,
	int line)
{
	int finite = 0;
	bool hasError = true;

	while (hasError && finite < 10)
	{
		GLenum err = glGetError();
		hasError = err != GL_NO_ERROR;

		if (hasError)
		{
			const char* str = GetErrorString(err);
			log_render("GL ERROR: %s in file %s[%d] %s", str, fname, line, stmt);
		}

		finite += 1;
	}
	
	return hasError;
}

#ifdef wGL_DEBUG
#	define gl(stmt)  stmt; if (MessageCallback(#stmt, __FILE__, __LINE__)) {}
#	define gle(stmt) stmt; if (MessageCallback(#stmt, __FILE__, __LINE__)) { err = true; }
#else
#	define gl(stmt)  stmt
#	define gle(stmt) stmt
#endif