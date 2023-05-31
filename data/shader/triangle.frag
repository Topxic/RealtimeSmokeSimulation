#version 330 core

in vec3 color;

out vec4 fragColor;

uniform float time;

void main()
{
    fragColor = vec4(time / 30 * color, 1.0);
}
