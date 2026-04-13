#version 450

#define TRANSPARENCY_EARLY_EXIT_THRES 1e-3
#define MAX_RAY_STEPS 100
#define EXTINCTION_MULT 0.1
#define RR_MIN_SAMPLES 50
#define RR_PROBABILITY 0.5

in vec3 pos;
in vec3 norm;
in vec2 uv;

out vec4 fragColor;

uniform vec3 cameraPos;
uniform float cameraFoV;

uniform vec3 cloudColor;
uniform float absorption;
uniform float border;
uniform int maxDensitySamples;
uniform float sampleStepSize;
uniform float time;
uniform bool showPerlin;
uniform bool showVoronoi;
uniform float showTextureDepth;

uniform float lightIntensity;
uniform vec3 lightPosition;
uniform vec3 lightColor;
uniform float henyeyGreen_G;
uniform float henyeyGreen_K;

uniform float exposure;
uniform float gamma;

uniform sampler3D voronoiTex;
uniform sampler3D perlinTex;

#define PI 3.14159
const vec3 cuboidSize = vec3(1, 1, 1);
const vec3 wind1 = vec3(0.06, 0.0, 0.02);
const vec3 wind2 = vec3(0.12, 0.0, -0.08);

float rnd(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

vec3 toneMapper(vec3 color) {
    // exposure tone mapping
    vec3 mapped = vec3(1.0) - exp(-color * exposure);
    // gamma correction 
    mapped = pow(mapped, vec3(1.0 / gamma));
  
    return mapped;
}

// Ray-box intersection: get exit position of ray from cuboid
vec3 getCuboidExitPos(vec3 origin, vec3 dir, vec3 cuboidSize) {
    vec3 t1 = (-cuboidSize - origin) / dir;
    vec3 t2 = ( cuboidSize - origin) / dir; 
    vec3 tMax = max(t1, t2);
    float tExit = min(tMax.x, min(tMax.y, tMax.z));
    return origin + tExit * dir;
}

float sampleFBM(vec3 pos) {
    vec3 uvw = pos + 0.5;
    return 1.0 - texture(voronoiTex, uvw).r; 
}

float samplePerlin(vec3 pos) {
    vec3 uvw = pos + 0.5;
    return texture(perlinTex, uvw).r; 
}

float sampleDensity(vec3 pos) {
    float voronoi = sampleFBM(pos + wind1 * time);
    float perlin  = samplePerlin(pos + wind2 * time);

    vec3 borderDistances = 0.5 - abs(pos);
    float borderFading = min(min(borderDistances.x, borderDistances.y), borderDistances.z);
    borderFading = smoothstep(0.0, border, borderFading);

    return borderFading * borderFading * perlin * voronoi;
}

float henyeyGreenstein(float theta, float g) {
    float denom = 1.0 + g * g - 2.0 * g * cos(theta);
    return (1.0 - g * g) / (sqrt(denom * denom * denom) * (4.0 * PI));
}

float beersLaw(float dist) {
    return exp(-dist * absorption);
}

float powderEffect(float dist) {
    return 1.0 - beersLaw(dist);
}

// https://www.desmos.com/calculator/qtkg1q1q3w?lang=de
float beersPowder(float dist) {
    return beersLaw(dist) * powderEffect(dist);
}

float multipleOctaveScattering(float density, float mu) {
    float attenuation = 0.2;
    float contribution = 0.4;
    float phaseAttenuation = 0.1;

    const float scatteringOctaves = 4.0;

    float a = 1.0;
    float b = 1.0;
    float c = 1.0;
    float g = 0.85;

    float luminance = 0.0;

    for (float i = 0.0; i < scatteringOctaves; i++) {
        float phaseFunction = henyeyGreenstein(0.3 + c, mu);
        float beers = exp(-density * EXTINCTION_MULT * a);

        luminance += b * phaseFunction * beers;

        a *= attenuation;
        b *= contribution;
        c *= (1.0 - phaseAttenuation);
    }

    return luminance;
}

bool russianRoulette(int idx, vec2 seed) {
    return !(idx < RR_MIN_SAMPLES || rnd(seed) < RR_PROBABILITY);
}

void main() {
    // Compute ray direction from camera to current fragment
    vec3 start = pos;
    vec3 dir = normalize(start - cameraPos);
    start += 1e-5 * dir;
    vec3 stop = getCuboidExitPos(start, dir, cuboidSize);

    // Sample along the ray
    int numSteps = int(length(stop - start) / sampleStepSize);
    if (numSteps <= 0) {
        discard;
    }

    vec3 color = vec3(0);
    float cloudTransparency = 1.0;
    for (int i = 0; i < numSteps && cloudTransparency > TRANSPARENCY_EARLY_EXIT_THRES && i < MAX_RAY_STEPS; ++i) {

        // Random jitter inside step range to prevent banding
        float outerRandom = rnd(vec2(rnd(gl_FragCoord.xy), float(i)));
        float t = (float(i) + (outerRandom - 0.5) * 0.5) * sampleStepSize;
        vec3 samplePos = start + dir * t;
        float cloudDensity = sampleStepSize * sampleDensity(samplePos);
        cloudTransparency *= 1.0 - beersPowder(cloudDensity);

        // Sample illumination along ray to light source
        vec3 lightRayDir = normalize(lightPosition - samplePos);
        vec3 lightRayStop = getCuboidExitPos(samplePos + 1e-3 * lightRayDir, lightRayDir, cuboidSize);
        float lightDistance = distance(lightRayStop, samplePos);
        int numLightSteps = int(lightDistance / sampleStepSize); // TODO Make step size of light samples configurable
        float lightTransparency = 1.0;
        for (int j = 0; j < numLightSteps && lightTransparency > TRANSPARENCY_EARLY_EXIT_THRES && j < MAX_RAY_STEPS; ++j) {
            float innerRandom = rnd(vec2(rnd(gl_FragCoord.xy), float(j)));
            t = (float(j) + (innerRandom - 0.5) * 0.5) * sampleStepSize;
            vec3 lightSamplePos = samplePos + t * lightRayDir; // TODO Here too
            float lightDensity = sampleStepSize * sampleDensity(lightSamplePos);
            lightTransparency *= 1.0 - multipleOctaveScattering(lightDensity, henyeyGreen_G);

            // Heuristic early ray stop optimization
            if(russianRoulette(j, vec2(outerRandom, innerRandom))) {
                break;
            }
        }    

        float cosTheta = acos(abs(dot(lightRayDir, -dir)));
        // Include backwards scattering component
        float phase = mix(
            henyeyGreenstein(cosTheta, henyeyGreen_G), 
            henyeyGreenstein(cosTheta, -henyeyGreen_G),
            henyeyGreen_K);
        // Weight by distance
        float lightRayDist = distance(lightPosition, samplePos);

        color += lightIntensity 
               * lightTransparency 
               * phase 
               * 1.0 / (lightRayDist * lightRayDist)
               * lightColor 
               * cloudTransparency 
               * cloudColor;

        // Heuristic early ray stop optimization
        if(russianRoulette(i, vec2(outerRandom, outerRandom))) {
            break;
        }
    }

    float opacity = 1.0 - cloudTransparency;
    

    if (showPerlin || showVoronoi) {
        vec3 samplePos = start + showTextureDepth * (stop - start);
        opacity = 1.0;
        color = vec3(0);
        if (showVoronoi)
            color += vec3(sampleFBM(samplePos));
        if (showPerlin)
            color += vec3(samplePerlin(samplePos));
    }


    color = toneMapper(color);
    fragColor = vec4(color, opacity);
}
