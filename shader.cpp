#include "shader.h"

#include <fstream>
#include <vector>
#include <iostream>

Shader::Shader(const std::string& vFile, const std::string& fFile)
    : uniformCache()
{

    //Initialize shaders
    vertSource = getShaderSourceFromFile(vFile);
    vertShader = loadShader(vertSource, GL_VERTEX_SHADER);
    fragSource = getShaderSourceFromFile(fFile);
    fragShader = loadShader(fragSource, GL_FRAGMENT_SHADER);

    //Link shaders to program
    program = glCreateProgram();
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);
    glLinkProgram(program);

    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    if (linkStatus == GL_FALSE)
    {
        //Get length of error report
        GLint logLen;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);

        //Store error in vector of characters
        std::vector<GLchar> log;
        glGetProgramInfoLog(program, logLen, &logLen, &log[0]);

        //Print out the error one character at a time
        std::cout << "Error in shader compilation!" << std::endl;
        for (GLchar& c : log)
        {
            std::cout << (char) c;
        }
        std::cout << std::endl;

        glDeleteProgram(program);
    }
    else
    {
        //glDetachShader(program, vertShader);
        //glDetachShader(program, fragShader);
    }

    glDeleteShader(vertShader);
    glDeleteShader(fragShader);
}
Shader::~Shader()
{
    glDeleteProgram(program);
}

void Shader::bind()
{
    glUseProgram(program);
}

GLuint Shader::loadShader(const std::string& source, const GLenum type)
{
    GLuint shader = glCreateShader(type);
    const char* const src = source.c_str();
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);

    GLint compileStatus;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);

    //Handle shader compilation error
    if (compileStatus == GL_FALSE)
    {
        //Get length of error string
        GLint logLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);

        //Put string into vector
        GLchar info [logLen];
        glGetShaderInfoLog(shader, logLen, &logLen, info);

        //Print out the error one character at a time
        std::cout << "Error in shader compilation!" << std::endl;
        for (GLchar& c : info)
        {
            std::cout << c;
        }
        std::cout << std::endl;

        glDeleteShader(shader);

        return -1;
    }

    return shader;
}

std::string Shader::getShaderSourceFromFile(const std::string& filePath)
{
    std::string output;

    std::ifstream stream(filePath);

    if (!stream.is_open())
    {
        perror("Failed to load shader file! ");
    }

    for (std::string line; std::getline(stream, line); )
    {
        output.append(line);
        output.append("\n");
    }

    if (stream.bad())
    {
        perror("Failed to read shader file contents! ");
    }

    stream.close();

    return output;
}


GLint Shader::getUniformLoc(const char* name)
{
    // return getUniformLoc(std::string(name));
    return glGetUniformLocation(program, name);
}