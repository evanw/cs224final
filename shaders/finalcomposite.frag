uniform sampler2D texture;
uniform vec2 windowSize;

void main()
{
    // recover normal and depth, reject if background
    vec2 coord = gl_FragCoord.xy / windowSize;
    vec4 normalDepth = texture2D(texture, coord);
    vec3 normal = normalDepth.xyz;
    float depth = normalDepth.w;
    if (depth == 0.0)
    {
        // color background differently
        gl_FragColor = vec4(0.5 * (1.0 - length(coord - 0.5)));
        return;
    }

    // change material in creases
    vec2 dx = vec2(1.0 / windowSize.x, 0.0);
    vec2 dy = vec2(0.0, 1.0 / windowSize.y);
    vec3 xneg = texture2D(texture, coord - dx).xyz;
    vec3 yneg = texture2D(texture, coord - dy).xyz;
    vec3 xpos = texture2D(texture, coord + dx).xyz;
    vec3 ypos = texture2D(texture, coord + dy).xyz;
    float curvature = (cross(xneg, xpos).y - cross(yneg, ypos).x) * 4.0 / depth;

#if MATERIAL == 0
    // direct curvature visualization
    vec3 light = vec3(0.0);
    vec3 ambient = vec3(curvature + 0.5);
    vec3 diffuse = vec3(0.0);
    vec3 specular = vec3(0.0);
    float shininess = 0.0;
#elif MATERIAL == 1
    // maple candy shader
    curvature = clamp(0.5 + curvature * 2.0, 0.0, 1.0);
    vec3 light = normalize(vec3(0.0, 1.0, 10.0));
    vec3 ambient = vec3(0.1, 0.05, 0.0);
    vec3 diffuse = mix(vec3(0.5), vec3(0.5, 0.3, 0.1), curvature);
    vec3 specular = vec3(0.0);
    float shininess = 0.0;
#elif MATERIAL == 2
    // metal shader
    float corrosion = clamp(-curvature * 3.0, 0.0, 1.0);
    float shine = clamp(curvature * 5.0, 0.0, 1.0);
    vec3 light = normalize(vec3(0.0, 1.0, 10.0));
    vec3 ambient = vec3(0.15, 0.1, 0.1);
    vec3 diffuse = mix(mix(vec3(0.3, 0.25, 0.2), vec3(0.45, 0.5, 0.5), corrosion), vec3(0.5, 0.4, 0.3), shine) - ambient;
    vec3 specular = mix(vec3(0.0), vec3(1.0) - ambient - diffuse, shine);
    float shininess = 128.0;
#elif MATERIAL == 3
    // red wax shader
    float dirt = clamp(0.25 - curvature * 4.0, 0.0, 1.0);
    vec3 light = normalize(vec3(0.0, 1.0, 10.0));
    vec3 ambient = vec3(0.1, 0.05, 0.0);
    vec3 diffuse = mix(vec3(0.4, 0.15, 0.1), vec3(0.4, 0.3, 0.3), dirt) - ambient;
    vec3 specular = mix(vec3(0.15) - ambient, vec3(0.0), dirt);
    float shininess = 32.0;
#endif

    // calculate final color
    float cosAngle = dot(normal, light);
    gl_FragColor.rgb =
        ambient +
        diffuse * max(0.0, cosAngle) +
        specular * pow(max(0.0, cosAngle), shininess);
}
