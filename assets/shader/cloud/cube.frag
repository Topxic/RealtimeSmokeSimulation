#version 450

in vec3 pos;
in vec3 norm;
in vec2 uv;

out vec4 fragColor;

uniform vec3 cloudColor;
uniform vec3 cameraPos;
uniform float absorption;
uniform float border;
uniform int maxDensitySamples;
uniform float sampleStepSize;
uniform float time;
uniform bool showPerlin;
uniform bool showVoronoi;
uniform float showTextureDepth;

uniform sampler3D voronoiTex;
uniform sampler3D perlinTex;

const vec3 cuboidSize = vec3(1, 1, 1);
const vec3 wind1 = vec3(0.06, 0.0, 0.02);
const vec3 wind2 = vec3(0.12, 0.0, -0.08);

// Ray-box intersection: get exit position of ray from cuboid
vec3 getCuboidExitPos(vec3 origin, vec3 dir, vec3 cuboidSize) {
    vec3 t1 = (-cuboidSize - origin) / dir;
    vec3 t2 = ( cuboidSize - origin) / dir; 
    vec3 tMax = max(t1, t2);
    float tExit = min(tMax.x, min(tMax.y, tMax.z));
    return origin + tExit * dir;
}

float sampleVoronoi(vec3 pos) {
    vec3 uvw = pos + 0.5;
    return 1.0 - texture(voronoiTex, uvw).r; 
}

float samplePerlin(vec3 pos) {
    vec3 uvw = pos + 0.5;
    return texture(perlinTex, uvw).r; 
}

void main() {
    // Compute ray direction from camera to current fragment
    vec3 start = pos;
    vec3 dir = normalize(start - cameraPos);
    vec3 stop = getCuboidExitPos(start + 1e-3 * dir, dir, cuboidSize);

    // Sample along the ray
    int numSteps = int(length(stop - start) / sampleStepSize);
    // Check if start overshot stop
    if (dot(dir, normalize(stop - start)) < 0.0) {
        numSteps = 0;
    }
    float accumulatedDensity = 0.0;
    for (int i = 0; i < numSteps; ++i) {

        vec3 samplePos = start + dir * (float(i) * sampleStepSize);
        float voronoi = sampleVoronoi(samplePos + wind1 * time);
        float perlin  = samplePerlin(samplePos + wind2 * time);

        float density = perlin + voronoi;
        vec3 borderDistances = 0.5 - abs(samplePos);
        float minBorder = 2.0 * min(min(borderDistances.x, borderDistances.y), borderDistances.z);
        minBorder = smoothstep(0.0, border, minBorder);

        accumulatedDensity += minBorder * density * sampleStepSize;
    }
    float opacity = 1.0 - exp(-absorption * accumulatedDensity);
    vec3 color = cloudColor;

    if (showPerlin || showVoronoi) {
        vec3 pos = start + showTextureDepth * (stop - start);
        opacity = 1.0;
        color = vec3(0);
        if (showVoronoi)
            color += vec3(sampleVoronoi(pos + wind1 * time));
        if (showPerlin)
            color += vec3(samplePerlin(pos + wind1 * time));
    }

    fragColor = vec4(color, opacity);
}
