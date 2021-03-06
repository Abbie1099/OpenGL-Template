#include<GL/glew.h>
#include <GLFW/glfw3.h>
#include<iostream>
#include<fstream>
#include<string>
#include<sstream>
#include"benchmark.h"

#define ASSERT(x) if(!(x)) __debugbreak();
#define GLCall(x) GLClearError();\
    x;\
ASSERT(GLLogCall(#x,__FILE__,__LINE__))

static void GLClearError()
{
    while (glGetError() != GL_NO_ERROR);
}
static bool GLLogCall(const char* function,const char* file,int line)
{
    while (GLenum error = glGetError())
    {
        std::cout << "OPENGL ERROR " << error << std::endl;
        return false;
    }
    return true;
}
struct ShaderProgramSource
{
    std::string VertexSource;
    std::string FragmentSource;
};
static ShaderProgramSource ParseShader(const std::string& filepath)
{
    std::ifstream stream(filepath);

    enum class ShaderType
    {
        none = -1, vertex = 0, fragment = 1
    };

    std::string line;
    std::stringstream ss[2];
    ShaderType type = ShaderType::none;
    while (std::getline(stream, line))
    {
        if(line.find("#shader") != std::string::npos)
            {
            if (line.find("vertex") != std::string::npos)
                type = ShaderType::vertex;
            else if (line.find("fragment") != std::string::npos)
                type = ShaderType::fragment;
            }
        else
        {
            ss[int(type)] << line << "\n";
        }
    }
    return { ss[0].str(),ss[1].str() };
}
static unsigned int CompileShader(const std::string& source, unsigned int type)
{
    unsigned int id = glCreateShader(GL_VERTEX_SHADER);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);
    int res;
    glGetShaderiv(id, GL_COMPILE_STATUS, &res);
    
        if (res == GL_FALSE)
        {
            int length;
            glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
            char* message = new char[length];
            glGetShaderInfoLog(id, length, &length, message);
            std::cout << "FAILED TO COMPILE SHADER" << std::endl;
            std::cout << (type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT") << std::endl;
            std::cout << message << std::endl;
            glDeleteShader(id);
            delete[] message;
            return 0;
        }
    return id;
}
static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(vertexShader, GL_VERTEX_SHADER);
    unsigned int fs = CompileShader(fragmentShader, GL_FRAGMENT_SHADER);
   
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);
    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}
int main(void)
{
    
    GLFWwindow* window;

    if (!glfwInit())
        return -1;

    window = glfwCreateWindow(800, 600, "OPEN GL", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }
  
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glewInit();
    std::cout <<"OPENGL DRIVER VERSION " << glGetString(GL_VERSION) << std::endl;
    float positions[] =
    {
        -0.5f, -0.5f,
         0.5f, -0.5f,
         0.5f, 0.5f,
        -0.5f, 0.5f,
    };
    unsigned int indices[] =
    {
        0,1,2,
        2,3,0,
    };
    unsigned int buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, 4 *2* sizeof(float),positions,GL_STATIC_DRAW);

    unsigned int Indexbuffer;
    glGenBuffers(1, &Indexbuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Indexbuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);
    ShaderProgramSource source = ParseShader("res/shader/Basic.shader");
    unsigned int shader = CreateShader(source.VertexSource, source.FragmentSource);
    glUseProgram(shader);

    int location = glGetUniformLocation(shader, "u_Color");
    //ASSERT(location != -1);
    
    float r = 0.0f;
    float increment = 0.5f;
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glUniform4f(location, r, 0.3f, 0.8f, 1.0f);
        GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));
        if (r > 1.0f)
        {
            increment = -0.05f;
        }
        else if (r < 0.0f)
        {
            increment = 0.05f;
        }
        r += increment;
        glfwSwapBuffers(window);

        glfwPollEvents();
        glFlush();
    }

    glfwTerminate();
    glDeleteProgram(shader);
    return 0;
}