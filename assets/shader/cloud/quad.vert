#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNorm;
layout (location = 2) in vec2 inTC;

out vec3 pos;
out vec3 norm;
out vec2 tc;

void main() {
    pos = inPos;
    norm = inNorm;
    tc = inTC;
    gl_Position = vec4(inPos, 1);
}
