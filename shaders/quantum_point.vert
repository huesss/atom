#version 330 core
layout(location = 0) in vec3 aDummy;
layout(location = 1) in vec4 iPosDen;
layout(location = 2) in vec4 iColorPhase;

uniform mat4 uView;
uniform mat4 uProj;
uniform float uPointScale;

out vec3 vColor;
out float vPhase;
out float vDensity;

void main() {
    vec4 eye = uView * vec4(iPosDen.xyz, 1.0);
    gl_Position = uProj * eye;
    float dist = max(0.32, -eye.z);
    gl_PointSize = clamp(uPointScale * (0.52 + 0.98 * iPosDen.w) / dist, 2.0, 88.0);
    vColor = iColorPhase.rgb;
    vPhase = iColorPhase.w;
    vDensity = iPosDen.w;
}
