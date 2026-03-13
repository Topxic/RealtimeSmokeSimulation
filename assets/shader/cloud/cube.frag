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

uniform sampler3D voronoiTex;

const vec3 cuboidSize = vec3(1, 1, 1);

#define FK(k) floatBitsToInt(cos(k))^floatBitsToInt(k)
float hash(float a, float b) {
    int x = FK(a); int y = FK(b);
    return float((x * x + y) * (y * y - x) + x) / 2.14e9;
}
vec3 hash3(float a) {
    float h = hash(a, a + 32.23);
    return vec3(hash(a, h), hash(a + h, h * a), hash(a + h, h));
}

// Ray-box intersection: get exit position of ray from cuboid
vec3 getCuboidExitPos(vec3 origin, vec3 dir, vec3 cuboidSize) {
    vec3 t1 = (-cuboidSize - origin) / dir;
    vec3 t2 = ( cuboidSize - origin) / dir; 
    vec3 tMax = max(t1, t2);
    float tExit = min(tMax.x, min(tMax.y, tMax.z));
    return origin + tExit * dir;
}

float sampleVoronoi(vec3 pos) {
    vec3 uvw = pos + 0.5 + vec3(
        0.01 * sin(5.5 * time),
        0.01 * sin(4.5 * time),
        0.01 * sin(1.5 * time)
    );
    return texture(voronoiTex, uvw).r; 
}

void main() {
    // Compute ray direction from camera to current fragment
    vec3 start = pos;
    vec3 stop = getCuboidExitPos(cameraPos, normalize(cameraPos - start), cuboidSize);
    vec3 dir = normalize(stop - start);

    // Sample along the ray
    int numSteps = int(length(stop - start) / sampleStepSize);
    float accumulatedDensity = 0.0;
    for (int i = 0; i < numSteps; ++i) {
        vec3 samplePos = start + dir * (float(i) * sampleStepSize);
        float voronoi = 1.0 - sampleVoronoi(samplePos);
        accumulatedDensity += voronoi * voronoi * sampleStepSize;
    }

    float opacity = 1.0 - exp(-accumulatedDensity * absorption);

    fragColor = vec4(cloudColor, opacity);
}
