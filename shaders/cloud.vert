#version 330 core
layout(location = 0) in vec3 aDummy;
layout(location = 1) in vec4 iPosDen;
layout(location = 2) in vec4 iPhasePack;

uniform mat4 uMVP;
uniform float uPointSize;

out float vDensity;
out float vPhase;

void main() {
    gl_Position = uMVP * vec4(iPosDen.xyz, 1.0);
    gl_PointSize = uPointSize;
    vDensity = iPosDen.w;
    vPhase = iPhasePack.x;
}
