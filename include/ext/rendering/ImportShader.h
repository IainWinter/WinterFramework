#pragma once

#include "Rendering.h"
#include <sstream>

//  Import a shader from a source file
//
r<ShaderProgram> ImportShader(const char* filename);

//
// detail
//

struct ShaderSource
{
	std::vector<std::pair<ShaderProgram::ShaderName, std::string>> source;
};

// Open a file and read all of its contents into a string 
// that gets processed by @ProcessShaderSource
//
ShaderSource LoadShaderSourceFromFile(const char* filename);

//	Convert a string of glsl into shader source
//
//	There are some preprocessor like functions this adds
// 
//	#include <filename> 
//		Copies the source from another file into this one, 
//		that file can have more imports, but no sections
// 
//	#section <vertex|fragment|geometry> 
//		Marks sections of a shader in one file
//		this allows for a whole program in one file
//
ShaderSource ProcessShaderSource(const char* stringSource);

//	Create a shader from shader source (only on the host until Use is called)
//
r<ShaderProgram> CreateShaderProgramFromSource(const ShaderSource& source);

class ShaderConsumer
{
public:
    ShaderConsumer(const char* current);

    ShaderSource GetSource() const;

private:
    std::unordered_map<ShaderProgram::ShaderName, std::stringstream> buffers;
    std::stringstream* currentBuffer;
    const char* current;

    void ConsumeShaderSource();
    void ConsumePreprocessor();

    void NextCh(int step = 1);

    bool IsChar(char c) const;
    bool IsEOF() const;
    bool IsWhitespace() const;
    bool IsNewline() const;
    int IsString(const char* str) const;

    void Emit();
    void Emit(char c);
    void Emit(const char* string);
};
