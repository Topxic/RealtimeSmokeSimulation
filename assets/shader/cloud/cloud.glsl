#ifndef CLOUD_GLSL
#define CLOUD_GLSL

#include "math.glsl"

#define RR_MIN_SAMPLES 50
#define RR_PROBABILITY 0.5
#define EXTINCTION_MULT 0.1

uniform vec3 cloudColor;
uniform float absorption;
uniform float border;
uniform int maxDensitySamples;
uniform float sampleStepSize;
uniform bool showFBM;
uniform bool showVoronoi;
uniform float showTextureDepth;

uniform float lightIntensity;
uniform vec3 lightPosition;
uniform vec3 lightColor;
uniform float henyeyGreen_G;
uniform float henyeyGreen_K;

uniform sampler3D voronoiTex;
uniform sampler3D fbmTex;

const vec3 wind1 = vec3(0.06, 0.0, 0.02);
const vec3 wind2 = vec3(0.12, 0.0, -0.08);

float sampleFBM(vec3 pos) {
    vec3 uvw = pos + 0.5;
    return 1.0 - texture(voronoiTex, uvw).r; 
}

float samplePerlin(vec3 pos) {
    vec3 uvw = pos + 0.5;
    return texture(fbmTex, uvw).r; 
}

float sampleDensity(vec3 pos, float time) {
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

#endif // CLOUD_GLSL