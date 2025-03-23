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

    // Color
    vec3 background = vec3(0.088, 0.084, 0.084);
    vec3 smoke = vec3(0);
    vec3 velocity = vec3(0);
    vec3 pressure = vec3(0);
    float transmittance = 1;

    for (int i = 0; i < 100000; i++) {

        float alpha, m, u, v, w, p;
        if (interpolate) 
        {
            // TODO How should be interpolated and how do I get rid of the visible voxel boundaries
            vec3 voxelCenter = vec3(voxelID) * gridSpacing;

            // Smoke 
            m = sampleField(voxelCenter.x, voxelCenter.y, voxelCenter.z, M_FIELD);
            alpha = thickness * (1 - m);
    
            // Velocity
            u = sampleField(voxelCenter.x, voxelCenter.y, voxelCenter.z, U_FIELD);
            v = sampleField(voxelCenter.x, voxelCenter.y, voxelCenter.z, V_FIELD);
            w = sampleField(voxelCenter.x, voxelCenter.y, voxelCenter.z, W_FIELD);
            
            // Pressure
            p = sampleField(voxelCenter.x, voxelCenter.y, voxelCenter.z, P_FIELD);
        } else {
            // Smoke 
            m = loadField(voxelID.x, voxelID.y, voxelID.z, M_FIELD);
            alpha = thickness * (1 - m);

            // Velocity
            u = loadField(voxelID.x, voxelID.y, voxelID.z, U_FIELD);
            v = loadField(voxelID.x, voxelID.y, voxelID.z, V_FIELD);
            w = loadField(voxelID.x, voxelID.y, voxelID.z, W_FIELD);

            // Pressure
            p = loadField(voxelID.x, voxelID.y, voxelID.z, P_FIELD);
        }

        smoke += alpha;
        velocity += alpha * vec3(u, v, w);
        pressure += (1 - m) * p;

        // Obstacle
        float s = loadField(voxelID.x, voxelID.y, voxelID.z, S_FIELD);
        if (s < 1.f) {
            smoke += transmittance * vec3(0);
            velocity += transmittance * vec3(0);
            pressure += transmittance * vec3(0);
            break;
        } 
        
        // Stop if we reach the exit voxel with a small threshold for precision
        if (voxelID == stopID) {
            smoke += transmittance * background;
            velocity += transmittance * background;
            pressure += transmittance * background;
            break;
        }

        // Change transmittance
        transmittance *= (1 - alpha);

        // Perform DDA step
        bvec3 mask = lessThan(tMax.xyz, tMax.yzx);
        bvec3 move;
        move.x = mask.x && !mask.z;
        move.y = mask.y && !mask.x;
        move.z = !(move.x || move.y);
        voxelID += ivec3(vec3(move) * raySign);
        tMax += vec3(move) * tDelta;
    }

    vec3 color = smoke;
    if (showVelocityField) {
        color = velocity;
    }

    if (showPressureField) {
        color = pressure;
    }

    // Output final voxel color
    fragColor = vec4(color, 1.0);
}
