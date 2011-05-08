varying vec3 pos;
varying vec3 normal;

void main()
{
    pos = (gl_ModelViewMatrix * gl_Vertex).xyz;
    normal = gl_NormalMatrix * gl_Normal;
    gl_Position = ftransform();
}
