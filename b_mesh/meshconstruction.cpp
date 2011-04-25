#include "meshconstruction.h"

void MeshConstruction::BMeshInit(Mesh *m){
    //empty the mesh
    m->vertices.clear();
    m->quads.clear();
    m->triangles.clear();

    //find the root node
    Ball *root = NULL;
    foreach(Ball b, m->balls){
        if(b.parentIndex == -1){
            root = &b;
            break;
        }
    }

    sweep(m, root);
    m->updateNormals();
}

void MeshConstruction::sweep(Mesh *m, const Ball *b){
    //world vectors
    Vector3 X = Vector3(1,0,0);
    Vector3 Y = Vector3(0,1,0);
    Vector3 Z = Vector3(0,0,1);

    if(b->childrenIndices.size() > 0){
        foreach(int i, b->childrenIndices){
            //sweep the child
            sweep(m, &m->balls.at(i));
            //the last quad on the mesh
        }
    } else if(b->childrenIndices.size() == 0){
        //this is an end node. find the local vectors
        Ball parent = m->balls.at(b->parentIndex);
        Vector3 boneDirection = parent.center - b->center;
        Vector3 x,y,z;
        x = -boneDirection.unit();
        if(x == Z){
            y = X;
            z = Y;
        }else {
            y = Z.cross(x);
            z = x.cross(y);
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
        Quad q0, q1, q2, q3;
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

        m->quads.push_back(q0);
        m->quads.push_back(q1);
        m->quads.push_back(q2);
        m->quads.push_back(q3);
    }
}

void MeshConstruction::stitch(Mesh *m){

}
