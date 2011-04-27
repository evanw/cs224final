#include "meshconstruction.h"

void MeshConstruction::BMeshInit(Mesh *m){

    //empty the mesh
    m->vertices.clear();
    m->quads.clear();
    m->triangles.clear();

    //call sweep at each root node
    foreach(Ball b, m->balls){
        if(b.parentIndex == -1){
            sweep(m, &b);
        }
    }

    m->updateNormals();
}

Quad MeshConstruction::sweep(Mesh *m, const Ball *b){
    //world vectors
    Vector3 X = Vector3(1,0,0);
    Vector3 Y = Vector3(0,1,0);
    Vector3 Z = Vector3(0,0,1);

    if(b->childrenIndices.size()>1){
        for(int i=0; i<b->childrenIndices.size(); i++){
            Quad lastQuad = sweep(m, &m->balls.at(b->childrenIndices.at(i)));
            Vector3 childDirection = m->balls.at(b->childrenIndices.at(i)).center - b->center;

            //find local axes
            Vector3 x,y,z;
            x = childDirection.unit();
            if(x.apequal(Y) || x.apequal(-Y)){
                y = X;
                z = Z;
            } else{
                y = Y.cross(x).unit();
                z = x.cross(y).unit();
            }

            //sweep the old vertices down the bone, and find their midpoint
            Vertex v0, v1, v2, v3;
            v0.pos = (m->vertices.at(lastQuad.a.index).pos +
                      m->vertices.at(lastQuad.b.index).pos +
                      m->vertices.at(lastQuad.c.index).pos +
                      m->vertices.at(lastQuad.d.index).pos)/4.0 - childDirection;
            v1.pos = v0.pos;
            v2.pos = v0.pos;
            v3.pos = v0.pos;

            //now push them back out to the radius of the sphere
            float r = b->maxRadius();
            x*=r;
            y*=r;
            z*=r;
            v0.pos += y;
            v0.pos += z;
            v1.pos -= y;
            v1.pos += z;
            v2.pos -= y;
            v2.pos -= z;
            v3.pos += y;
            v3.pos -= z;

            //push them to the tangent plane to the sphere
            v0.pos += x;
            v1.pos += x;
            v2.pos += x;
            v3.pos += x;

            //add these vertices
            int startIndex = m->vertices.size();
            m->vertices.push_back(v0);
            m->vertices.push_back(v1);
            m->vertices.push_back(v2);
            m->vertices.push_back(v3);

            //make indices for all these vertices
            Index i0, i1, i2, i3;
            i0.index = startIndex;
            i1.index = startIndex +1;
            i2.index = startIndex +2;
            i3.index = startIndex +3;

            //now sweep
            Quad q0, q1, q2, q3, toReturn;
            q0.d = lastQuad.a.index;
            q0.c = lastQuad.b.index;
            q0.b = i1;
            q0.a = i0;
            q1.d = lastQuad.b.index;
            q1.c = lastQuad.c.index;
            q1.b = i2;
            q1.a = i1;
            q2.d = lastQuad.c.index;
            q2.c = lastQuad.d.index;
            q2.b = i3;
            q2.a = i2;
            q3.d = lastQuad.d.index;
            q3.c = lastQuad.a.index;
            q3.b = i0;
            q3.a = i3;
            toReturn.a = i0;
            toReturn.b = i1;
            toReturn.c = i2;
            toReturn.d = i3;

            m->quads.push_back(q0);
            m->quads.push_back(q1);
            m->quads.push_back(q2);
            m->quads.push_back(q3);
        }
        Quad toReturn;
        return toReturn;
    }else if(b->childrenIndices.size() == 1){
        //sweep the child
        Quad lastQuad = sweep(m, &m->balls.at(b->childrenIndices.at(0)));
        Vector3 childDirection = m->balls.at(b->childrenIndices.at(0)).center - b->center;
        Vector3 parentDirection = m->balls.at(b->parentIndex).center - b->center;
        Vector3 rotationAxis = childDirection.cross(parentDirection).unit();
        float rotationAngle = -acos(-childDirection.dot(parentDirection) / childDirection.length() / parentDirection.length()) / 2.0;

        cout<<rotationAxis<<rotationAngle<<endl;

        //find local axes
        Vector3 x,y,z;
        x = childDirection.unit();
        if(x.apequal(Y) || x.apequal(-Y)){
            y = X;
            z = Z;
        } else{
            y = Y.cross(x).unit();
            z = x.cross(y).unit();
        }

        //sweep the old vertices down the bone, and find their midpoint
        Vertex v0, v1, v2, v3;
        v0.pos = (m->vertices.at(lastQuad.a.index).pos +
                  m->vertices.at(lastQuad.b.index).pos +
                  m->vertices.at(lastQuad.c.index).pos +
                  m->vertices.at(lastQuad.d.index).pos)/4.0 - childDirection;
        v1.pos = v0.pos;
        v2.pos = v0.pos;
        v3.pos = v0.pos;

        //now push them back out to the radius of the sphere
        float r = b->maxRadius();
        x*=r;
        y*=r;
        z*=r;
        v0.pos += y;
        v0.pos += z;
        v1.pos -= y;
        v1.pos += z;
        v2.pos -= y;
        v2.pos -= z;
        v3.pos += y;
        v3.pos -= z;

        //rotate them around the connection node
        v0.pos = rotate(v0.pos, rotationAxis, b->center, rotationAngle);
        v1.pos = rotate(v1.pos, rotationAxis, b->center, rotationAngle);
        v2.pos = rotate(v2.pos, rotationAxis, b->center, rotationAngle);
        v3.pos = rotate(v3.pos, rotationAxis, b->center, rotationAngle);

        //add these vertices
        int startIndex = m->vertices.size();
        m->vertices.push_back(v0);
        m->vertices.push_back(v1);
        m->vertices.push_back(v2);
        m->vertices.push_back(v3);

        //make indices for all these vertices
        Index i0, i1, i2, i3;
        i0.index = startIndex;
        i1.index = startIndex +1;
        i2.index = startIndex +2;
        i3.index = startIndex +3;

        //now add the sweep to the center of the sphere
        Quad q0, q1, q2, q3, toReturn;
        q0.d = lastQuad.a.index;
        q0.c = lastQuad.b.index;
        q0.b = i1;
        q0.a = i0;
        q1.d = lastQuad.b.index;
        q1.c = lastQuad.c.index;
        q1.b = i2;
        q1.a = i1;
        q2.d = lastQuad.c.index;
        q2.c = lastQuad.d.index;
        q2.b = i3;
        q2.a = i2;
        q3.d = lastQuad.d.index;
        q3.c = lastQuad.a.index;
        q3.b = i0;
        q3.a = i3;
        toReturn.a = i0;
        toReturn.b = i1;
        toReturn.c = i2;
        toReturn.d = i3;

        m->quads.push_back(q0);
        m->quads.push_back(q1);
        m->quads.push_back(q2);
        m->quads.push_back(q3);

        return toReturn;

    } else if(b->childrenIndices.size() == 0){
        //this is an end node. find the local vectors
        Ball parent = m->balls.at(b->parentIndex);
        Vector3 boneDirection = parent.center - b->center;
        Vector3 x,y,z;
        x = -boneDirection.unit();
        if(x.apequal(Y) || x.apequal(-Y)){
            y = X;
            z = Z;
        }else {
            y = Y.cross(x).unit();
            z = x.cross(y).unit();
        }

        float r = b->maxRadius();
        x*=r;
        y*=r;
        z*=r;

        //make the quad cap
        Vertex v0, v1, v2, v3, v4, v5, v6, v7;
        v4.pos = b->center;
        v5.pos = b->center;
        v6.pos = b->center;
        v7.pos = b->center;

        //these verts are the center of the sphere
        v4.pos += y;
        v4.pos += z;
        v5.pos -= y;
        v5.pos += z;
        v6.pos -= y;
        v6.pos -= z;
        v7.pos += y;
        v7.pos -= z;
        //these are the edge of the sphere
        v0.pos = v4.pos + x;
        v1.pos = v5.pos + x;
        v2.pos = v6.pos + x;
        v3.pos = v7.pos + x;

        //add the cap vertices to the mesh, keep track of the start index
        int startIndex = m->vertices.size();
        m->vertices.push_back(v0);
        m->vertices.push_back(v1);
        m->vertices.push_back(v2);
        m->vertices.push_back(v3);
        m->vertices.push_back(v4);
        m->vertices.push_back(v5);
        m->vertices.push_back(v6);
        m->vertices.push_back(v7);

        //make indices for all these vertices
        Index i0, i1, i2, i3, i4, i5, i6, i7;
        i0.index = startIndex;
        i1.index = startIndex +1;
        i2.index = startIndex +2;
        i3.index = startIndex +3;
        i4.index = startIndex +4;
        i5.index = startIndex +5;
        i6.index = startIndex +6;
        i7.index = startIndex +7;

        //add the quad to the cap
        Quad cap;
        cap.a = i0;
        cap.b = i1;
        cap.c = i2;
        cap.d = i3;
        m->quads.push_back(cap);

        //now add the sweep to the center of the sphere
        Quad q0, q1, q2, q3, toReturn;
        q0.a = i4;
        q0.b = i5;
        q0.c = i1;
        q0.d = i0;
        q1.a = i5;
        q1.b = i6;
        q1.c = i2;
        q1.d = i1;
        q2.a = i6;
        q2.b = i7;
        q2.c = i3;
        q2.d = i2;
        q3.a = i7;
        q3.b = i4;
        q3.c = i0;
        q3.d = i3;
        toReturn.a = i4;
        toReturn.b = i5;
        toReturn.c = i6;
        toReturn.d = i7;

        m->quads.push_back(q0);
        m->quads.push_back(q1);
        m->quads.push_back(q2);
        m->quads.push_back(q3);

        return toReturn;
    }
}

void MeshConstruction::stitch(Mesh *m){

}


Vector3 MeshConstruction::rotate(const Vector3 &p, const Vector3 &v, const Vector3 &c, float radians){

    Vector3 temp = Vector3(p);
    temp -= c;

    Vector3 axisX = Vector3(1, 1, 1).cross(v).unit();
    Vector3 axisY = v.cross(axisX);

    float x = temp.dot(axisX);
    float y = temp.dot(axisY);
    float z = temp.dot(v);
    float sinA = sin(radians);
    float cosA = cos(radians);
    Vector3 toReturn = axisX * (x * cosA - y * sinA) + axisY * (y * cosA + x * sinA) + v * z;

    return toReturn + c;

}
