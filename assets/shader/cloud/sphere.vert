#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNorm;
layout (location = 2) in vec2 inTC;

out vec3 pos;
out vec3 norm;
out vec2 uv;

uniform mat4 PV;
uniform vec3 lightPosition;

void main() {
    pos = inPos + lightPosition;
    norm = inNorm;
    uv = inTC;
    gl_Position = PV * vec4(0.1 * pos, 1);
}
