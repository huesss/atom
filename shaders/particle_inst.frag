#version 330 core
in float vLight;
in vec3 vColor;
in float vPhase;
out vec4 FragColor;

uniform float uTime;

void main() {
    float glow = pow(vLight, 2.0);
    vec3 base = vColor * (0.4 + 0.6 * glow);
    float ph = vPhase + uTime * 0.4;
    vec3 tint = vec3(0.06 * sin(ph), 0.05 * sin(ph + 2.1), 0.06 * sin(ph + 4.2));
    vec3 col = clamp(base + tint, 0.0, 1.0);
    FragColor = vec4(col, 0.94);
}
