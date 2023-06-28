#include "data/shader/smoke/smokeHeader.glsl"

in vec2 tc;
out vec4 fragColor;

void main() {
  vec2 id = h * tc * gridResolution;
  ivec2 iid = ivec2(tc * gridResolution);
  vec3 color = vec3(0);

  float s, u, v, p, m;
  if (interpolate) {
    s = sampleField(id.x, id.y, S_FIELD);
    u = sampleField(id.x, id.y, U_FIELD);
    v = sampleField(id.x, id.y, V_FIELD);
    p = sampleField(id.x, id.y, P_FIELD);
    m = sampleField(id.x, id.y, M_FIELD);
  } else {
    s = loadField(iid.x, iid.y, S_FIELD);
    u = loadField(iid.x, iid.y, U_FIELD);
    v = loadField(iid.x, iid.y, V_FIELD);
    p = loadField(iid.x, iid.y, P_FIELD);
    m = loadField(iid.x, iid.y, M_FIELD);
  }
    
  if (s < 0.8f) {
    color = vec3(0, 0, s);
  } else if (showVelocityField) { 
    color = vec3(u, v, 0);
  } else if (showPressureField) {
    color = vec3(p / 30000000);
  } else {
    color =  vec3(m);
  }

  fragColor = vec4(color, 1);
}
