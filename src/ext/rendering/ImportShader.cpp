#include "ext/rendering/ImportShader.h"
#include "util/filesystem.h"

#include <fstream>
#include <sstream>

ShaderSource LoadShaderSourceFromFile(const char* filename)
{
    std::ifstream file(filename);
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    std::string contents = buffer.str();

    return ProcessShaderSource(contents.c_str());
}

ShaderSource ProcessShaderSource(const char* stringSource)
{
    ShaderConsumer consumer(stringSource);
    return consumer.GetSource();
}

r<ShaderProgram> CreateShaderProgramFromSource(const ShaderSource& source)
{
    r<ShaderProgram> shader = mkr<ShaderProgram>();

    for (const auto [name, source] : source.source)
    {
        shader->Add(name, source);
    }

    return shader;
}

//
//  Shader Consumer class
//

ShaderConsumer::ShaderConsumer(const char* current)
    : current (current)
{
    currentBuffer = &buffers[ShaderProgram::sVertex];
    ConsumeShaderSource();
}

ShaderSource ShaderConsumer::GetSource() const
{
    ShaderSource source;

    for (const auto& [name, buffer] : buffers)
    {
        source.source.push_back(std::make_pair(name, buffer.str()));
    }
        
    return source;
}

void ShaderConsumer::ConsumeShaderSource()
{
    while (!IsEOF())
    {
        switch (*current)
        {
            case '#':
                ConsumePreprocessor();
                break;

            //case ';':
            //    Emit(";\n");
            //    NextCh();
            //    break;

            default:
                Emit();
                NextCh();
                break;
        }
    }
}

void ShaderConsumer::ConsumePreprocessor()
{
    NextCh();

    // extract string from #(a..za..z*)( |<)arguments>

    const char* beginPreprocess = current;

    while (!IsEOF() && !IsWhitespace())
    {
        NextCh();
    }

    std::string preprocess = std::string(beginPreprocess, current);

    while (IsWhitespace()) // allow for whitespace between preprocessor and arguments
    {
        NextCh();
    }

    const char* beginArguments = current;

    while (!IsEOF() && !IsNewline()) // the arguments are everything before a newline
    {
        NextCh();
    }

    std::string arguments = std::string(beginArguments, current);

    printf("Import shader found preprocessor: %s %s\n", preprocess.c_str(), arguments.c_str());

    if (preprocess == "include")
    {
        // recurse into another ShaderConsumer
        // make the assumption that there is only going to be one type of source in this sub shader

        Emit("\n//");
        Emit("\n// Begin include from ");
        Emit(arguments.c_str());
        Emit("\n//");
        Emit('\n');

        ShaderSource source = LoadShaderSourceFromFile(_a(arguments).c_str());
        Emit(source.source.front().second.c_str());

        Emit("\n//");
        Emit("\n// End include from ");
        Emit(arguments.c_str());
        Emit("\n//");
        Emit('\n');
    }

    else
    if (preprocess == "section")
    {
        // switch to another section

        ShaderProgram::ShaderName name;

             if (arguments == "vertex")   name = ShaderProgram::sVertex;
        else if (arguments == "fragment") name = ShaderProgram::sFragment;
        else if (arguments == "geometry") name = ShaderProgram::sGeometry;
        else if (arguments == "compute")  name = ShaderProgram::sCompute;
        else
        {
            //log error and return
            return;
        }

        // could put a warning that there are two of the same sections
            
        currentBuffer = &buffers[name];

        // consume newline
        NextCh();
    }

    else // for all other preprocessors, just emit them
    {
        Emit('#');
        Emit(preprocess.c_str());
        Emit(' ');
        Emit(arguments.c_str());
    }
}

void ShaderConsumer::NextCh(int step)
{
    current += step;
}

bool ShaderConsumer::IsChar(char c) const
{
    return *current == c;
}

bool ShaderConsumer::IsEOF() const
{
    return *current == EOF
        || *current == '\0'; // added \0 for loaded files from fstream -> string
}

bool ShaderConsumer::IsWhitespace() const
{
    return *current == ' '
        || *current == '\t'
        || IsNewline();
}

bool ShaderConsumer::IsNewline() const
{
    return *current == '\n'
        //|| *current == '\r\n'
        || *current == '\r';
}

int ShaderConsumer::IsString(const char* str) const
{
    int length = (int)strlen(str);
    for (int i = 0; i < length; i++)
    {
        if (current[i] != str[i])
            return 0;
    }

    return length;
}

void ShaderConsumer::Emit()
{
    Emit(*current);
}

void ShaderConsumer::Emit(char c)
{
    *currentBuffer << c;
}

void ShaderConsumer::Emit(const char* string)
{
    *currentBuffer << string;
}
