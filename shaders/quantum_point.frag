#version 330 core
in vec3 vColor;
in float vPhase;
in float vDensity;
out vec4 FragColor;

uniform float uTime;

void main() {
    vec2 q = gl_PointCoord * 2.0 - 1.0;
    float r = length(q);
    if (r > 1.0) discard;
    float t = r * r;
    float halo = exp(-t * 2.4);
    float core = exp(-t * 11.0);
    vec3 cool = vec3(0.2, 0.35, 0.82);
    vec3 col = max(vColor, cool * 0.45);
    col = col * (0.72 + 0.55 * halo);
    col = mix(col, vec3(0.75, 0.88, 1.0), 0.22 * core);
    float ph = vPhase + uTime * 0.45;
    col += 0.12 * vec3(sin(ph), sin(ph + 2.09), sin(ph + 4.18));
    col = clamp(col, 0.0, 1.0);
    float a = halo * (0.42 + 0.58 * vDensity);
    a = min(a * 1.15, 0.92);
    FragColor = vec4(col, a);
}
