#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inTC;

out vec2 tc;

void main() {
    tc = inTC;
    gl_Position = vec4(inPos, 1);
}
