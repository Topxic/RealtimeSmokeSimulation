#version 450

in vec3 pos;
in vec3 norm;
in vec2 uv;

out vec4 fragColor;

uniform vec3 cloudColor;
uniform vec3 cameraPos;
uniform float absorption;
uniform int maxDensitySamples;
uniform float sampleStepSize;
uniform float time;

uniform sampler2D voronoiTex;

const vec3 cuboidSize = vec3(1, 1, 1);

// Ray-box intersection: get exit position of ray from cuboid
vec3 getCuboidExitPos(vec3 origin, vec3 dir, vec3 cuboidSize) {
    vec3 t1 = (-cuboidSize - origin) / dir;
    vec3 t2 = ( cuboidSize - origin) / dir; 
    vec3 tMax = max(t1, t2);
    float tExit = min(tMax.x, min(tMax.y, tMax.z));
    return origin + tExit * dir;
}

float sampleRandomNoise(vec3 pos) {
    return absorption; 
}

void main() {
    // Compute ray direction from camera to current fragment
    vec3 rayDir = normalize(pos - cameraPos);
    vec3 start = cameraPos;
    vec3 stop = getCuboidExitPos(cameraPos, rayDir, cuboidSize);

    vec3 dir = normalize(stop - start);

    // Step along the ray
    int numSteps = int(length(stop - start) / sampleStepSize);

    float accumulatedDensity = 0.0;
    for (int i = 0; i < numSteps; ++i) {
        vec3 samplePos = start + dir * (float(i) * sampleStepSize);
        accumulatedDensity += sampleRandomNoise(samplePos) * sampleStepSize;
    }

    float opacity = 1.0 - exp(-accumulatedDensity * absorption);
    vec3 color = cloudColor;

    color = 1.0 - texture(voronoiTex, uv).rgb;

    fragColor = vec4(color, 1.0);
}