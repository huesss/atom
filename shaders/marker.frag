#version 330 core
out vec4 FragColor;
uniform vec3 uColor;

void main() {
    vec2 d = gl_PointCoord * 2.0 - 1.0;
    if (dot(d, d) > 1.0) discard;
    FragColor = vec4(uColor, 1.0);
}
