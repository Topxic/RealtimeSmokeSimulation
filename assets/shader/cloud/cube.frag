#version 450

in vec3 pos;
in vec3 norm;

out vec4 fragColor;

uniform vec3 cloudColor;
uniform vec3 cameraPos;
uniform float absorption;

vec3 getCuboidExitPos(vec3 origin, vec3 dir, vec3 cuboidSize) {
    vec3 t1 = (-cuboidSize - origin) / dir;
    vec3 t2 = ( cuboidSize - origin) / dir; 
    vec3 tMax = max(t1, t2);
    float tExit = min(tMax.x, min(tMax.y, tMax.z));
    return origin + tExit * dir;
}

void main() {
    vec3 rayDir = normalize(pos - cameraPos);

    // Compute world space ray entry and exit positions
    vec3 start = pos;
    vec3 stop = getCuboidExitPos(cameraPos, rayDir, vec3(1, 1, 1));

    float beersLaw = exp(-absorption * distance(start, stop));
    vec3 color = beersLaw * cloudColor;

    // Output final voxel color
    fragColor = vec4(color, 1.0);
}
