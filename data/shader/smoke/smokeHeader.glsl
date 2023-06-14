#version 450

/* Staggered velocity grid layout      
 *  ______ ______ ______ 
 * |  v0  |  v1  |  v2  | 
 * u0     u1     u2     u3
 * |__v3__|__v4__|__v5__| 
 * |      |      |      | 
 * u4     u5     u6     u7
 * |__v6__|__v7__|__v8__| 
 */

#define U_FIELD 0
#define V_FIELD 1
#define NEXT_U_FIELD 2
#define NEXT_V_FIELD 3
#define S_FIELD 4
#define P_FIELD 5
#define M_FIELD 6
#define NEXT_M_FIELD 7

layout(std430, binding = 0) buffer velocityFieldU {
    float velocityU[];
};

layout(std430, binding = 1) buffer velocityFieldV {
    float velocityV[];
};

layout(std430, binding = 2) buffer nextVelocityFieldU {
    float nextVelocityU[];
};

layout(std430, binding = 3) buffer nextVelocityFieldV {
    float nextVelocityV[];
};

layout(std430, binding = 4) buffer obstacleField {
    float obstacle[];
};

layout(std430, binding = 5) buffer pressureField {
    float pressure[];
};

layout(std430, binding = 6) buffer smokeField {
    float smoke[];
};

layout(std430, binding = 7) buffer nextSmokeField {
    float nextSmoke[];
};

uniform vec2 gridResolution;
uniform vec2 screenResolution;
uniform float dt; // delta time 
uniform float h; // Grid spacing
uniform float overrelaxation;
uniform vec2 gravity;
uniform float density;
uniform int alternatingBit;
uniform int iterations;
uniform bool showVelocityField;
uniform bool showPressureField;
uniform bool interpolate;
uniform bool reset;

const float maxVelocity = 100.f;

float loadField(int x, int y, int field) {
    if (x < 0 || x >= int(gridResolution.x) || y < 0 || y >= int(gridResolution.y)) {
        return 0.f;
    }
    int i = x * int(gridResolution.y) + y;
    switch (field) {
        case U_FIELD:
            return velocityU[i];
        case V_FIELD:
            return velocityV[i];
        case NEXT_U_FIELD:
            return nextVelocityU[i];
        case NEXT_V_FIELD:
            return nextVelocityV[i];
        case S_FIELD:
            return obstacle[i];
        case P_FIELD:
            return pressure[i];
        case M_FIELD:
            return smoke[i];
        case NEXT_M_FIELD:
            return nextSmoke[i];
    }
    return 0.f;
}

void saveField(int x, int y, int field, float value) {
    if (x < 0 || x >= int(gridResolution.x) || y < 0 || y >= int(gridResolution.y)) {
        return;
    }
    int i = x * int(gridResolution.y) + y;
    switch (field) {
        case U_FIELD:
            velocityU[i] = value;
            return;
        case V_FIELD:
            velocityV[i] = value;
            return;
        case NEXT_U_FIELD:
            nextVelocityU[i] = value;
            return;
        case NEXT_V_FIELD:
            nextVelocityV[i] = value;
            return;
        case S_FIELD:
            obstacle[i] = value;
            return;
        case P_FIELD:
            pressure[i] = value;
            return;
        case M_FIELD:
            smoke[i] = value;
            return;
        case NEXT_M_FIELD:
            nextSmoke[i] = value;
            return;
    }
}

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

float sampleField(float x, float y, int field) {
    float h1 = 1.f / h;
    float h2 = h / 2.f;

    x = clamp(x, h, h * int(gridResolution.x));
    y = clamp(y, h, h * int(gridResolution.y));
    float dx = 0.f;
    float dy = 0.f;

    switch (field) {
        case U_FIELD:
        case NEXT_U_FIELD:
            dy = h2;
            break;

        case V_FIELD:
        case NEXT_V_FIELD:
            dx = h2;
            break;

        case S_FIELD:
        case P_FIELD:
        case M_FIELD:
        case NEXT_M_FIELD:
            dx = h2;
            dy = h2;
            break;
    }

    float x0 = min(floor((x - dx) * h1), int(gridResolution.x) - 1);
    float tx = h1 * ((x - dx) - x0 * h);
    float x1 = min(x0 + 1.f, int(gridResolution.x) - 1);

    float y0 = min(floor((y - dy) * h1), int(gridResolution.y) - 1);
    float ty = h1 * ((y - dy) - y0 * h);
    float y1 = min(y0 + 1.f, int(gridResolution.y) - 1);

    float sx = 1.f - tx;
    float sy = 1.f - ty;

    return sx * sy * loadField(int(x0), int(y0), field) +
           tx * sy * loadField(int(x1), int(y0), field) +
           tx * ty * loadField(int(x1), int(y1), field) +
           sx * ty * loadField(int(x0), int(y1), field);
}