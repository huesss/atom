#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec4 iWorld;
layout(location = 2) in vec4 iColorPhase;

uniform mat4 uView;
uniform mat4 uProj;
uniform float uRadius;

out float vLight;
out vec3 vColor;
out float vPhase;

void main() {
    vec3 wp = iWorld.xyz + aPos * uRadius;
    vec3 n = normalize(aPos);
    vec3 lightDir = normalize(vec3(1.0, 1.0, 0.72));
    vLight = max(dot(n, lightDir), 0.36);
    vColor = iColorPhase.rgb;
    vPhase = iColorPhase.w;
    gl_Position = uProj * uView * vec4(wp, 1.0);
}
