#version 450

#include "cloud.glsl"
#include "camera.glsl"

#define TRANSPARENCY_EARLY_EXIT_THRES 1e-3
#define MAX_RAY_STEPS 100

#define RAYMARCHING_MIN_DIST 1e-3
#define RAYMARCHING_MAX_DIST 1e+3
#define RAYMARCHING_MAX_STEPS 100

in vec3 pos;
in vec3 norm;
in vec2 tc;

out vec4 fragColor;

uniform float dt;
uniform float time;

uniform float exposure;
uniform float gamma;

const vec3 cuboidSize = vec3(1, 1, 1);


vec3 toneMapper(vec3 color) {
    // exposure tone mapping
    vec3 mapped = vec3(1.0) - exp(-color * exposure);
    // gamma correction 
    mapped = pow(mapped, vec3(1.0 / gamma));
  
    return mapped;
}





float map(vec3 p) {
    return sdBox(p, vec3(1));
}

void main() {
    vec3 rayDir = spawnRay(tc);

    float t = 0.0;
    bool hit = false;
    for (int i = 0; i < RAYMARCHING_MAX_STEPS && t < RAYMARCHING_MAX_DIST; ++i) {

        vec3 rayPos = cameraEye + rayDir * t;
        float dist = map(rayPos);

        if (dist < RAYMARCHING_MIN_DIST) {
            hit = true;
            break;
        }
        
        t += dist;
    }

    if (hit) {
        fragColor = vec4(cameraEye + rayDir * t, 1.0);
    } else {
        fragColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
    
    return;

    // Compute ray direction from camera to current fragment
    vec3 rayStart;
    vec3 rayStop;
    if (!intersectCuboid(cameraEye, rayDir, vec3(0.5), rayStart, rayStop)) {
        discard;
    }
    rayStart += 1e-5 * rayDir;

    

    // Sample along the ray
    int numSteps = int(length(rayStop - rayStart) / sampleStepSize);
    if (numSteps <= 0) {
        discard;
    }

    vec3 color = vec3(0);
    float cloudTransparency = 1.0;
    for (int i = 0; i < numSteps && cloudTransparency > TRANSPARENCY_EARLY_EXIT_THRES && i < MAX_RAY_STEPS; ++i) {

        // Random jitter inside step range to prevent banding
        float outerRandom = rnd(vec2(rnd(gl_FragCoord.xy), float(i)));
        float t = (float(i) + (outerRandom - 0.5) * 0.5) * sampleStepSize;
        vec3 samplePos = rayStart + rayDir * t;
        float cloudDensity = sampleStepSize * sampleDensity(samplePos, time);
        cloudTransparency *= 1.0 - beersPowder(cloudDensity);

        // Sample illumination along ray to light source
        vec3 lightRayDir = normalize(lightPosition - samplePos);
        vec3 lightRayStart = samplePos + 1e-3 * lightRayDir;
        vec3 lightRayStartDiscard; // TODO 
        vec3 lightRayStop;
        if (!intersectCuboid(lightRayStart, lightRayDir, vec3(0.5), lightRayStartDiscard, lightRayStop)) {
            continue;
        }
        float lightDistance = distance(lightRayStop, samplePos);
        int numLightSteps = int(lightDistance / sampleStepSize); // TODO Make step size of light samples configurable
        float lightTransparency = 1.0;
        for (int j = 0; j < numLightSteps && lightTransparency > TRANSPARENCY_EARLY_EXIT_THRES && j < MAX_RAY_STEPS; ++j) {
            float innerRandom = rnd(vec2(rnd(gl_FragCoord.xy), float(j)));
            t = (float(j) + (innerRandom - 0.5) * 0.5) * sampleStepSize;
            vec3 lightSamplePos = samplePos + t * lightRayDir; // TODO Here too
            float lightDensity = sampleStepSize * sampleDensity(lightSamplePos, time);
            lightTransparency *= 1.0 - multipleOctaveScattering(lightDensity, henyeyGreen_G);

            // Heuristic early ray stop optimization
            if(russianRoulette(j, vec2(outerRandom, innerRandom))) {
                break;
            }
        }    

        float cosTheta = acos(abs(dot(lightRayDir, -rayDir)));
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
    

    if (showFBM || showVoronoi) {
        vec3 samplePos = rayStart + showTextureDepth * (rayStop - rayStart);
        opacity = 1.0;
        color = vec3(0);
        if (showVoronoi)
            color += vec3(sampleFBM(samplePos));
        if (showFBM)
            color += vec3(samplePerlin(samplePos));
    }


    color = toneMapper(color);
    fragColor = vec4(color, opacity);
}
