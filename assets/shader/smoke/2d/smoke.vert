#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inNorm;
layout (location = 2) in vec2 inTC;

out vec2 tc;

uniform mat4 PV;

void main() {
    tc = inTC;
    gl_Position = PV * vec4(inPos, 1);
}
