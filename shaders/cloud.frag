#version 330 core
in float vDensity;
in float vPhase;
out vec4 FragColor;

uniform float uTime;

void main() {
    vec2 d = gl_PointCoord * 2.0 - 1.0;
    if (dot(d, d) > 1.0) discard;
    float t = uTime * 0.5;
    vec3 baseColor = mix(vec3(0.05, 0.12, 0.95), vec3(0.95, 0.15, 0.55), vDensity);
    float ph = vPhase + t;
    vec3 phaseColor =
        vec3(0.5 + 0.5 * sin(ph), 0.5 + 0.5 * sin(ph + 2.094), 0.5 + 0.5 * sin(ph + 4.189));
    vec3 col = mix(baseColor, phaseColor, 0.4);
    float alpha = 0.12 + 0.7 * vDensity;
    FragColor = vec4(col, alpha);
}
