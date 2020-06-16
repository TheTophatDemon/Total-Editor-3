#pragma once

#include <glew.h>
#include <GL/gl.h>
#include <memory>
#include <SDL.h>
#include <unordered_map>

class Shader
{
public:
    Shader(const std::string& vFile, const std::string& fFile);
    ~Shader();
    void bind();
    //Retrieves the index of the shader's uniform variable with the given name
    GLint getUniformLoc(const char* name);
protected:
    std::unordered_map<std::string, GLint> uniformCache;
    std::string vertSource;
    std::string fragSource;
    GLuint vertShader;
    GLuint fragShader;
    GLuint program;
    //Returns a string representing the text in the file passed in
    std::string getShaderSourceFromFile(const std::string& filePath);
    //Creates and compiles a shader based on given source code and type
    GLuint loadShader(const std::string& source, const GLenum type);
};