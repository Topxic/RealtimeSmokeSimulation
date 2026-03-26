#version 450

in vec3 pos;
in vec3 norm;
in vec2 uv;

out vec4 fragColor;

uniform float lightIntensity;
uniform vec3 lightColor;

void main() {
    fragColor = vec4(lightIntensity * lightColor, 1);
}
