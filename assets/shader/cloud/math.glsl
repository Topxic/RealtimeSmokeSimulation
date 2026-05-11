#ifndef MATH_GLSL
#define MATH_GLSL

#define PI 3.14159

float rnd(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

bool intersectCuboid(vec3 origin, vec3 dir, vec3 cuboidSize, out vec3 entryPos, out vec3 exitPos) {
    vec3 halfSize = cuboidSize * 0.5;
    vec3 tMin = (-halfSize - origin) / dir;
    vec3 tMax = ( halfSize - origin) / dir;
    vec3 t1 = min(tMin, tMax);
    vec3 t2 = max(tMin, tMax);
    float tEntry = max(max(t1.x, t1.y), t1.z);
    float tExit  = min(min(t2.x, t2.y), t2.z);

    // No intersection
    if (tExit < 0.0 || tEntry > tExit)
        return false;

    entryPos = origin + tEntry * dir;
    exitPos  = origin + tExit  * dir;
    return true;
}

float sdSphere(vec3 p, float r) {
  return length(p) - r;
}

float sdBox(vec3 p, vec3 b) {
  vec3 q = abs(p) - b;
  return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0);
}

#endif // MATH_GLSL