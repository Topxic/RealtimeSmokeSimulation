#version 450

#define LOCAL_SIZE_X 8
#define LOCAL_SIZE_Y 8
#define LOCAL_SIZE_Z 16

#define U_FIELD 0
#define V_FIELD 1
#define W_FIELD 2
#define NEXT_U_FIELD 3
#define NEXT_V_FIELD 4
#define NEXT_W_FIELD 5
#define S_FIELD 6
#define P_FIELD 7
#define M_FIELD 8
#define NEXT_M_FIELD 9

layout(std430, binding = 0) buffer velocityFieldU {
    float velocityU[];
};

layout(std430, binding = 1) buffer velocityFieldV {
    float velocityV[];
};

layout(std430, binding = 2) buffer velocityFieldW {
    float velocityW[];
};

layout(std430, binding = 3) buffer nextVelocityFieldU {
    float nextVelocityU[];
};

layout(std430, binding = 4) buffer nextVelocityFieldV {
    float nextVelocityV[];
};

layout(std430, binding = 5) buffer nextVelocityFieldW {
    float nextVelocityW[];
};

layout(std430, binding = 6) buffer obstacleField {
    float obstacle[];
};

layout(std430, binding = 7) buffer pressureField {
    float pressure[];
};

layout(std430, binding = 8) buffer smokeField {
    float smoke[];
};

layout(std430, binding = 9) buffer nextSmokeField {
    float nextSmoke[];
};

uniform vec3 gridResolution;
uniform float dt; // delta time 
uniform float gridSpacing; // Grid spacing
uniform float overrelaxation;
uniform vec3 gravity;
uniform float density;
uniform int currentIteration;
uniform int totalIterations;
uniform bool showVelocityField;
uniform bool showPressureField;
uniform bool interpolate;
uniform bool reset;

const float maxVelocity = 100.f;

float loadField(int x, int y, int z, int field) {
    // Check bounds and calculate index
    int idx = -1;
    switch (field) {
        case U_FIELD:
        case NEXT_U_FIELD:
            if (x < 0 || x > int(gridResolution.x) || y < 0 || y >= int(gridResolution.y) || z < 0 || z >= int(gridResolution.z)) {
                return 0.f;
            }
            idx = z * int((gridResolution.x + 1) * gridResolution.y) + y * int(gridResolution.x + 1) + x;
            break;

        case V_FIELD:
        case NEXT_V_FIELD:
            if (x < 0 || x >= int(gridResolution.x) || y < 0 || y > int(gridResolution.y) || z < 0 || z >= int(gridResolution.z)) {
                return 0.f;
            }
            idx = z * int(gridResolution.x * (gridResolution.y + 1)) + x * int(gridResolution.y + 1) + y;
            break;

        case W_FIELD:
        case NEXT_W_FIELD:
            if (x < 0 || x >= int(gridResolution.x) || y < 0 || y >= int(gridResolution.y) || z < 0 || z > int(gridResolution.z)) {
                return 0.f;
            }
            idx = x * int(gridResolution.y * (gridResolution.z + 1)) + y * int(gridResolution.z + 1) + z;
            break;

        case S_FIELD:
        case P_FIELD:
            if (x < 0 || x >= int(gridResolution.x) || y < 0 || y >= int(gridResolution.y) || z < 0 || z >= int(gridResolution.z)) {
                return 0.f;
            }
            idx = z * int(gridResolution.x * gridResolution.y) + x * int(gridResolution.y) + y;
            break;

        case M_FIELD:
        case NEXT_M_FIELD:
            if (x < 0 || x >= int(gridResolution.x) || y < 0 || y >= int(gridResolution.y) || z < 0 || z >= int(gridResolution.z)) {
                return 1.f;
            }
            idx = z * int(gridResolution.x * gridResolution.y) + x * int(gridResolution.y) + y;
            break;
    }

    // Lookup value
    switch (field) {
        case U_FIELD:
            return velocityU[idx];
        case V_FIELD:
            return velocityV[idx];
        case W_FIELD:
            return velocityW[idx];
        case NEXT_U_FIELD:
            return nextVelocityU[idx];
        case NEXT_V_FIELD:
            return nextVelocityV[idx];
        case NEXT_W_FIELD:
            return nextVelocityW[idx];
        case S_FIELD:
            return obstacle[idx];
        case P_FIELD:
            return pressure[idx];
        case M_FIELD:
            return smoke[idx];
        case NEXT_M_FIELD:
            return nextSmoke[idx];
    }
    return 0.f;
}

void saveField(int x, int y, int z, int field, float value) {
    // Check bounds and calculate index
    int idx = -1;
    switch (field) {
        case U_FIELD:
        case NEXT_U_FIELD:
            if (x < 0 || x > int(gridResolution.x) || y < 0 || y >= int(gridResolution.y) || z < 0 || z >= int(gridResolution.z)) {
                return;
            }
            idx = z * int(gridResolution.y * (gridResolution.x + 1)) + y * int(gridResolution.x + 1) + x;
            break;

        case V_FIELD:
        case NEXT_V_FIELD:
            if (x < 0 || x >= int(gridResolution.x) || y < 0 || y > int(gridResolution.y) || z < 0 || z >= int(gridResolution.z)) {
                return;
            }
            idx = z * int(gridResolution.x * (gridResolution.y + 1)) + x * int(gridResolution.y + 1) + y;
            break;

        case W_FIELD:
        case NEXT_W_FIELD:
            if (x < 0 || x >= int(gridResolution.x) || y < 0 || y >= int(gridResolution.y) || z < 0 || z > int(gridResolution.z)) {
                return;
            }
            idx = y * int(gridResolution.x * (gridResolution.z + 1)) + x * int(gridResolution.z + 1) + z;
            break;

        case S_FIELD:
        case P_FIELD:
            if (x < 0 || x >= int(gridResolution.x) || y < 0 || y >= int(gridResolution.y) || z < 0 || z >= int(gridResolution.z)) {
                return;
            }
            idx = z * int(gridResolution.x * gridResolution.y) + x * int(gridResolution.y) + y;
            break;

        case M_FIELD:
        case NEXT_M_FIELD:
            if (x < 0 || x >= int(gridResolution.x) || y < 0 || y >= int(gridResolution.y) || z < 0 || z >= int(gridResolution.z)) {
                return;
            }
            idx = z * int(gridResolution.x * gridResolution.y) + x * int(gridResolution.y) + y;
            break;
    }

    // Save value
    switch (field) {
        case U_FIELD:
            velocityU[idx] = value;
            break;
        case V_FIELD:
            velocityV[idx] = value;
            break;
        case W_FIELD:
            velocityW[idx] = value;
            break;
        case NEXT_U_FIELD:
            nextVelocityU[idx] = value;
            break;
        case NEXT_V_FIELD:
            nextVelocityV[idx] = value;
            break;
        case NEXT_W_FIELD:
            nextVelocityW[idx] = value;
            break;
        case S_FIELD:
            obstacle[idx] = value;
            break;
        case P_FIELD:
            pressure[idx] = value;
            break;
        case M_FIELD:
            smoke[idx] = value;
            break;
        case NEXT_M_FIELD:
            nextSmoke[idx] = value;
            break;
    }
}

float sampleField(float x, float y, float z, int field) {
    float h = gridSpacing;
    float h1 = 1 / h;
    float h2 = h / 2;

    x = clamp(x, h, h * int(gridResolution.x));
    y = clamp(y, h, h * int(gridResolution.y));
    z = clamp(z, h, h * int(gridResolution.z));
    float dx = 0;
    float dy = 0;
    float dz = 0;

    switch (field) {
        case U_FIELD:
        case NEXT_U_FIELD:
            dy = h2;
            dz = h2;
            break;

        case V_FIELD:
        case NEXT_V_FIELD:
            dx = h2;
            dz = h2;
            break;

        case W_FIELD:
        case NEXT_W_FIELD:
            dx = h2;
            dy = h2;
            break;

        case S_FIELD:
        case P_FIELD:
        case M_FIELD:
        case NEXT_M_FIELD:
            dx = h2;
            dy = h2;
            dz = h2;
            break;
    }

    float x0 = min(floor((x - dx) * h1), int(gridResolution.x) - 1);
    float tx = h1 * ((x - dx) - x0 * h);
    float x1 = min(x0 + 1, int(gridResolution.x) - 1);

    float y0 = min(floor((y - dy) * h1), int(gridResolution.y) - 1);
    float ty = h1 * ((y - dy) - y0 * h);
    float y1 = min(y0 + 1, int(gridResolution.y) - 1);

    float z0 = min(floor((z - dz) * h1), int(gridResolution.z) - 1);
    float tz = h1 * ((z - dz) - z0 * h);
    float z1 = min(z0 + 1, int(gridResolution.z) - 1);

    float sx = 1 - tx;
    float sy = 1 - ty;
    float sz = 1 - tz;

    float c000 = loadField(int(x0), int(y0), int(z0), field);
    float c100 = loadField(int(x1), int(y0), int(z0), field);
    float c010 = loadField(int(x0), int(y1), int(z0), field);
    float c110 = loadField(int(x1), int(y1), int(z0), field);
    float c001 = loadField(int(x0), int(y0), int(z1), field);
    float c101 = loadField(int(x1), int(y0), int(z1), field);
    float c011 = loadField(int(x0), int(y1), int(z1), field);
    float c111 = loadField(int(x1), int(y1), int(z1), field);

    float c00 = sx * c000 + tx * c100;
    float c10 = sx * c010 + tx * c110;
    float c01 = sx * c001 + tx * c101;
    float c11 = sx * c011 + tx * c111;

    float c0 = sy * c00 + ty * c10;
    float c1 = sy * c01 + ty * c11;

    return sz * c0 + tz * c1;
}

/*
float avgU(int x, int y) {
    float u1 = loadField(x, y - 1, U_FIELD);
    float u2 = loadField(x, y, U_FIELD);
    float u3 = loadField(x + 1, y - 1, U_FIELD);
    float u4 = loadField(x + 1, y, U_FIELD);
    return (u1 + u2 + u3 + u4) / 4.f;
}

float avgV(int x, int y) {
    float v1 = loadField(x - 1, y, V_FIELD);
    float v2 = loadField(x, y, V_FIELD);
    float v3 = loadField(x - 1, y + 1, V_FIELD);
    float v4 = loadField(x, y + 1, V_FIELD);
    return (v1 + v2 + v3 + v4) / 4.f;
}


*/