#ifndef CAMERA_GLSL
#define CAMERA_GLSL

uniform vec3 cameraEye;
uniform vec3 cameraCenter;
uniform vec3 cameraUp;
uniform float fovRad;
uniform float aspect;

vec3 spawnRay(vec2 tc) {
    vec2 uv = tc * 2.0 - 1.0; // [0, 1] -> [-1, 1]
    uv.x *= aspect;
    vec3 forward = normalize(cameraCenter - cameraEye);
    vec3 left = normalize(cross(forward, cameraUp));
    vec3 up = -cross(left, forward); // USK is left-handed
    vec3 rayDir = normalize(
        forward +
        uv.x * tan(fovRad * 0.5) * left +
        uv.y * tan(fovRad * 0.5) * up
    );
   
    return rayDir;
}

#endif // CAMERA_GLSL