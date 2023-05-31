#version 330 core

in vec3 color;

out vec4 fragColor;

uniform float time;

void main()
{
    fragColor = vec4(time / 10 * color, 1.0);
}
