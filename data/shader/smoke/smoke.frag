#include "data/shader/smoke/smokeHeader.glsl"

in vec2 tc;
out vec4 fragColor;

void main() {
  vec2 id = h * tc * gridResolution;
  ivec2 iid = ivec2(tc * gridResolution);
  vec3 color = vec3(0.11);

  float s, u, v, p, m;
  s = loadField(iid.x, iid.y, S_FIELD);
  if (interpolate) {
    u = sampleField(id.x, id.y, U_FIELD);
    v = sampleField(id.x, id.y, V_FIELD);
    p = sampleField(id.x, id.y, P_FIELD);
    m = sampleField(id.x, id.y, M_FIELD);
  } else {
    u = loadField(iid.x, iid.y, U_FIELD);
    v = loadField(iid.x, iid.y, V_FIELD);
    p = loadField(iid.x, iid.y, P_FIELD);
    m = loadField(iid.x, iid.y, M_FIELD);
  }
    
  if (showVelocityField) { 
    color = abs(vec3(u, v, 0)) / maxVelocity;
  } else if (showPressureField) {
    color = vec3(p);
  } else if (m < .9f) {
    color = vec3(1 - m);
  }

  // Obstacle
  if (s < 1.f) {
    color = vec3(0);
  } 

  fragColor = vec4(color, 1);
}
