#include "smokeHeader.glsl"

in vec3 pos;
in vec3 norm;

out vec4 fragColor;

uniform vec3 cameraPos;

vec3 getCuboidExitPos(vec3 origin, vec3 dir, vec3 cuboidSize) {
    vec3 t1 = (-cuboidSize - origin) / dir;
    vec3 t2 = ( cuboidSize - origin) / dir; 
    vec3 tMax = max(t1, t2);
    float tExit = min(tMax.x, min(tMax.y, tMax.z));
    return origin + tExit * dir;
}

void main() {
    vec3 color = vec3(0.11);
    vec3 rayDir = normalize(pos - cameraPos);
    float h = 1 / max(gridResolution.x, max(gridResolution.y, gridResolution.z));
    vec3 cuboidSize = gridResolution * h;

    // Compute world space ray entry and exit positions
    vec3 start = pos;
    vec3 stop = getCuboidExitPos(cameraPos, rayDir, 0.5 * cuboidSize);

    // Convert world positions to voxel indices
    ivec3 startID = ivec3(
        clamp((start + 0.5 * cuboidSize) / h, vec3(0), gridResolution - 1)
    );
    ivec3 stopID = ivec3(
        clamp((stop + 0.5 * cuboidSize) / h, vec3(0), gridResolution - 1)
    );

    // Step direction: +1 or -1 depending on ray direction
    vec3 raySign = sign(rayDir);

    // How far we must move in each axis to cross one voxel
    vec3 tDelta = abs(h / rayDir);

    // Compute the next voxel boundary position
    vec3 nextVoxelBoundary = (startID + step(0, rayDir)) * h - 0.5 * cuboidSize;

    // Compute distance to next voxel boundary
    vec3 tMax = (nextVoxelBoundary - pos) / rayDir;


    // Traverse the grid using 3D DDA
    ivec3 voxelID = startID;
    vec3 velocity = vec3(0);
    int count = 0;
    for (int i = 0; i < 1000; i++) {

        if (interpolate) {
            // Smoke 
            color += 1 - sampleField(voxelID.x * gridSpacing, voxelID.y * gridSpacing, voxelID.z * gridSpacing, M_FIELD);
            // Velocity
            velocity.x += sampleField(voxelID.x * gridSpacing, voxelID.y * gridSpacing, voxelID.z * gridSpacing, U_FIELD);
            velocity.y += sampleField(voxelID.x * gridSpacing, voxelID.y * gridSpacing, voxelID.z * gridSpacing, V_FIELD);
            velocity.z += sampleField(voxelID.x * gridSpacing, voxelID.y * gridSpacing, voxelID.z * gridSpacing, W_FIELD);
        } else {
            // Smoke 
            color += 1 - loadField(voxelID.x, voxelID.y, voxelID.z, M_FIELD);
            // Velocity
            velocity.x += loadField(voxelID.x, voxelID.y, voxelID.z, U_FIELD);
            velocity.y += loadField(voxelID.x, voxelID.y, voxelID.z, V_FIELD);
            velocity.z += loadField(voxelID.x, voxelID.y, voxelID.z, W_FIELD);
        }

        // Obstacle
        float s = loadField(voxelID.x, voxelID.y, voxelID.z, S_FIELD);
        if (s < 1.f) {
            color = vec3(s);
            break;
        } 
        
        // Stop if we reach the exit voxel with a small threshold for precision
        count++;
        if (voxelID == stopID) {
            break;
        }

        // Perform DDA step
        bvec3 mask = lessThan(tMax.xyz, tMax.yzx);
        bvec3 move;
        move.x = mask.x && !mask.z;
        move.y = mask.y && !mask.x;
        move.z = !(move.x || move.y);
        voxelID += ivec3(vec3(move) * raySign);
        tMax += vec3(move) * tDelta;
    }

    velocity /= count;

    if (showVelocityField) {
        color = velocity;
    }

    // Output final voxel color
    fragColor = vec4(color, 1.0);
}
