varying vec3 pos;
varying vec3 normal;

void main()
{
    gl_FragColor = vec4(normalize(normal).xyz, length(pos));
}
