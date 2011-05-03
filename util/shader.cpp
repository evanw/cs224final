#include "shader.h"
#include <qgl.h>
#include <QFile>
#include <QTextStream>

inline string readFile(const string &path)
{
    QFile file(path.c_str());
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        cout << "error: could not open " << path << endl;
        exit(0);
    }
    QTextStream stream(&file);
    return stream.readAll().toStdString();
}

static void error(const string &type, const string &errorLog, const string &sourceCode = "")
{
    cout << "----- " << type << " -----" << endl;
    cout << errorLog << endl;
    if (!sourceCode.empty())
    {
        cout << "----- source code -----" << endl;
        cout << sourceCode << endl;
    }
    exit(0);
}

Shader::~Shader()
{
    glDeleteProgram(program);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void Shader::load(const string &vertexPath, const string &fragmentPath)
{
    int length;
    char buffer[512];
    const char *source;
    string vertexSource = readFile(vertexPath);
    string fragmentSource = readFile(fragmentPath);

    source = vertexSource.c_str();
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &source, NULL);
    glCompileShader(vertexShader);

    source = fragmentSource.c_str();
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &source, NULL);
    glCompileShader(fragmentShader);

    program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    glGetShaderInfoLog(vertexShader, sizeof(buffer), &length, buffer);
    if (length) error(vertexPath + " - compile error", buffer, vertexSource);

    glGetShaderInfoLog(fragmentShader, sizeof(buffer), &length, buffer);
    if (length) error(fragmentPath + " - compile error", buffer, fragmentSource);

    glGetProgramInfoLog(program, sizeof(buffer), &length, buffer);
    if (length) error(vertexPath + " - " + fragmentPath + " - link error", buffer);
}

void Shader::use()
{
    glUseProgram(program);
}

void Shader::unuse()
{
    glUseProgram(0);
}

void Shader::texture(const char *name, int value)
{
    glUniform1i(glGetUniformLocation(program, name), value);
}

void Shader::uniform(const char *name, float value)
{
    glUniform1f(glGetUniformLocation(program, name), value);
}

void Shader::uniform(const char *name, float x, float y)
{
    glUniform2f(glGetUniformLocation(program, name), x, y);
}

void Shader::uniform(const char *name, float x, float y, float z)
{
    glUniform3f(glGetUniformLocation(program, name), x, y, z);
}
