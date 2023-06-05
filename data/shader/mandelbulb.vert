#version 450

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTC;

out vec2 tc;

void main() {
    tc = aTC;
    gl_Position = vec4(aPos, 1);
}
