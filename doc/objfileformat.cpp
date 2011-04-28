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

void split(string str, const string &split, vector<string> &result)
{
    // trim characters in split from the start of str
    string::size_type start = str.find_first_not_of(split);
    if (start != string::npos) str = str.substr(start);

    // trim characters in split from the end of str
    string::size_type end = str.find_last_not_of(split);
    if (end != string::npos) str = str.substr(0, end + 1);

    // perform the split
    start = 0;
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
    if (!f.good()) return false;

    balls.clear();
    vertices.clear();
    triangles.clear();
    quads.clear();

    string line;
    Vector3 minVertex(FLT_MAX, FLT_MAX, FLT_MAX);
    Vector3 maxVertex(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    while (getline(f, line))
    {
        // remove the trailing newline
        if (!line.empty() && line[line.size() - 1] == '\r') line.erase(line.end() - 1);
        if (!line.empty() && line[line.size() - 1] == '\n') line.erase(line.end() - 1);

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
        else if (parts[0] == "j" && parts.size() == 14)
        {
            Ball ball;
            ball.center = Vector3(atof(parts[1].c_str()), atof(parts[2].c_str()), atof(parts[3].c_str()));
            ball.ex = Vector3(atof(parts[4].c_str()), atof(parts[5].c_str()), atof(parts[6].c_str()));
            ball.ey = Vector3(atof(parts[7].c_str()), atof(parts[8].c_str()), atof(parts[9].c_str()));
            ball.ez = Vector3(atof(parts[10].c_str()), atof(parts[11].c_str()), atof(parts[12].c_str()));
            ball.parentIndex = atoi(parts[13].c_str()) - 1;
            balls += ball;
        }
    }

    // scale the model to fit in a 4x4x4 cube
    // don't do this if there are any balls, because otherwise the skeleton would be misaligned
    if (balls.isEmpty())
    {
        float scale = 4 / max(-minVertex.min(), maxVertex.max());
        for (int i = 0; i < vertices.count(); i++)
        {
            Vertex &vertex = vertices[i];
            vertex.pos *= scale;
        }
    }

    // remove bad triangles
    for (int i = 0; i < triangles.size(); i++)
    {
        Triangle &tri = triangles[i];
        if (tri.a.index < 0 || tri.b.index < 0 || tri.c.index < 0 ||
                tri.a.index >= vertices.count() || tri.b.index >= vertices.count() || tri.c.index >= vertices.count())
            triangles.remove(i--);
    }

    // remove bad quads
    for (int i = 0; i < quads.size(); i++)
    {
        Quad &quad = quads[i];
        if (quad.a.index < 0 || quad.b.index < 0 || quad.c.index < 0 || quad.d.index < 0 ||
                quad.a.index >= vertices.count() || quad.b.index >= vertices.count() || quad.c.index >= vertices.count() || quad.d.index >= vertices.count())
            quads.remove(i--);
    }

    // remove bad balls
    for (int i = 0; i < balls.size(); i++)
    {
        Ball &ball = balls[i];
        if (ball.parentIndex < -1 || ball.parentIndex >= balls.size() || ball.parentIndex == i)
        {
            // subtract one from all parent indices > i and unlink all children
            for (int j = 0; j < balls.size(); j++)
            {
                Ball &ball = balls[j];
                if (ball.parentIndex == i)
                    ball.parentIndex = -1;
                else if (ball.parentIndex > i)
                    ball.parentIndex--;
            }

            // can now remove this ball
            balls.remove(i--);
        }
    }

    updateNormals();

    return true;
}

bool Mesh::saveToOBJ(const string &file)
{
    ofstream f(file.c_str());
    if (!f.good()) return false;

    foreach (const Vertex &vertex, vertices)
        f << "v " << vertex.pos.x << " " << vertex.pos.y << " " << vertex.pos.z << endl;

    foreach (const Triangle &tri, triangles)
        f << "f " << (tri.a.index + 1) << " " << (tri.b.index + 1) << " " << (tri.c.index + 1) << endl;

    foreach (const Quad &quad, quads)
        f << "f " << (quad.a.index + 1) << " " << (quad.b.index + 1) << " " << (quad.c.index + 1) << " " << (quad.d.index + 1) << endl;

    foreach (const Ball &ball, balls)
    {
        f << "j " << ball.center.x << " " << ball.center.y << " " << ball.center.z;
        f << " " << ball.ex.x << " " << ball.ex.y << " " << ball.ex.z;
        f << " " << ball.ey.x << " " << ball.ey.y << " " << ball.ey.z;
        f << " " << ball.ez.x << " " << ball.ez.y << " " << ball.ez.z;
        f << " " << (ball.parentIndex + 1) << endl;
    }

    return true;
}
