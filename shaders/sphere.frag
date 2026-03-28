#version 330 core
in vec3 vWorldPos;
in vec3 vN;
in float vLight;
out vec4 FragColor;

uniform vec3 uColor;
uniform vec3 uCamPos;

void main() {
    vec3 viewDir = normalize(uCamPos - vWorldPos);
    float ndv = max(dot(vN, viewDir), 0.0);
    float rim = pow(1.0 - ndv, 1.65);
    vec3 molten = vec3(1.0, 0.78, 0.35);
    vec3 crust = uColor * (0.38 + 0.62 * vLight);
    vec3 col = mix(crust, molten, 0.18 + 0.72 * rim);
    col += molten * 0.15 * rim * rim;
    FragColor = vec4(clamp(col, 0.0, 1.0), 1.0);
}
