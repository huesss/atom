#version 330 core
layout(location = 0) in vec3 aPos;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;

out vec3 vWorldPos;
out vec3 vN;
out float vLight;

void main() {
    vWorldPos = vec3(uModel * vec4(aPos, 1.0));
    vN = normalize(mat3(uModel) * aPos);
    vec3 lightDir = normalize(vec3(1.0, 1.0, 0.65));
    vLight = max(dot(vN, lightDir), 0.24);
    gl_Position = uProj * uView * uModel * vec4(aPos, 1.0);
}
