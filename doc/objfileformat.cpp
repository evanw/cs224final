#include "mesh.h"
#include <float.h>
#include <fstream>
#include <vector>
using namespace std;

inline int getInt(const string &str)
{
    string::size_type slash = str.find('/');
    return atoi((slash == string::npos ? str : str.substr(0, slash)).c_str());
}

void split(const string &str, const string &split, vector<string> &result)
{
    string::size_type start = 0, end;
    while ((end = str.find_first_of(split, start)) != string::npos)
    {
        result.push_back(str.substr(start, end - start));
        start = str.find_first_not_of(split, end);
        if (start == string::npos) return;
    }
    result.push_back(str.substr(start));
}

bool Mesh::loadFromOBJ(const string &file)
{
    ifstream f(file.c_str());
    if (f.bad()) return false;

    balls.clear();
    vertices.clear();
    triangles.clear();
    quads.clear();

    string line;
    Vector3 minVertex(FLT_MAX, FLT_MAX, FLT_MAX);
    Vector3 maxVertex(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    while (getline(f, line))
    {
        vector<string> parts;
        split(line, " ", parts);

        if (parts[0] == "v" && parts.size() == 4)
        {
            float x = atof(parts[1].c_str());
            float y = atof(parts[2].c_str());
            float z = atof(parts[3].c_str());
            Vector3 pos(x, y, z);
            minVertex = Vector3::min(minVertex, pos);
            maxVertex = Vector3::max(maxVertex, pos);
            vertices += Vertex(pos);
        }
        else if (parts[0] == "f" && parts.size() >= 4)
        {
            int a = getInt(parts[1]) - 1;
            int b = getInt(parts[2]) - 1;
            if (parts.size() == 5)
            {
                int c = getInt(parts[3]) - 1;
                int d = getInt(parts[4]) - 1;
                quads += Quad(a, b, c, d);
            }
            else
            {
                for (unsigned int i = 3; i < parts.size(); i++)
                {
                    int c = getInt(parts[i]) - 1;
                    triangles += Triangle(a, b, c);
                    b = c;
                }
            }
        }
    }

    // scale the model to fit in a 4x4x4 cube
    float scale = 4 / max(-minVertex.min(), maxVertex.max());
    for (int i = 0; i < vertices.count(); i++)
    {
        Vertex &vertex = vertices[i];
        vertex.pos *= scale;
    }

    // remove bad triangles
    for (int i = 0; i < triangles.size(); i++)
    {
        Triangle &tri = triangles[i];
        if (tri.a.index < 0 || tri.b.index < 0 || tri.c.index < 0 ||
                tri.a.index >= vertices.count() || tri.b.index >= vertices.count() || tri.c.index >= vertices.count())
            triangles.removeAt(i--);
    }

    updateNormals();

    return true;
}

bool Mesh::saveToOBJ(const string &file)
{
    ofstream f(file.c_str());
    if (f.bad()) return false;

    foreach (const Vertex &vertex, vertices)
        f << "v " << vertex.pos.x << " " << vertex.pos.y << " " << vertex.pos.z << endl;

    foreach (const Triangle &tri, triangles)
        f << "f " << (tri.a.index + 1) << " " << (tri.b.index + 1) << " " << (tri.c.index + 1) << endl;

    foreach (const Quad &quad, quads)
        f << "f " << (quad.a.index + 1) << " " << (quad.b.index + 1) << " " << (quad.c.index + 1) << " " << (quad.d.index + 1) << endl;

    return true;
}
