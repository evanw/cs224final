#pragma once

#include "vector.h"
#include <string>

using namespace std;

class Shader
{
private:
    unsigned int program;
    unsigned int vertexShader;
    unsigned int fragmentShader;

public:
    ~Shader();

    void init(const string &vertexPath, const string &fragmentPath, const string &defines = "");
    void use();
    void unuse();

    void texture(const char *name, int value);
    void uniform(const char *name, float value);
    void uniform(const char *name, float x, float y);
    void uniform(const char *name, float x, float y, float z);
    void uniform(const char *name, const Vector2 &value) { uniform(name, value.x, value.y); }
    void uniform(const char *name, const Vector3 &value) { uniform(name, value.x, value.y, value.z); }
};
