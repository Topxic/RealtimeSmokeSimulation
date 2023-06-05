#version 450

in vec2 tc;
out vec4 fragColor;

// Mandelbulb configuration
uniform int iterations;
uniform float bailout;
uniform float power;

// Raymarching configuration
uniform int maximalSteps;
uniform float maximalDistance;

// Light configuration
uniform vec3 lightDir;

uniform float time;
uniform vec2 resolution;
uniform vec3 cameraPosition;
uniform vec3 cameraDirection;
uniform bool antiAliasing;

vec2 rand(vec2 co){
    float r1 = fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
    float r2 = fract(sin(r1) * 43758.5453);
    return vec2(r1, r2);
}

vec3 hsv2rgb(vec3 hsv) {
  float h = hsv.x;
  float s = hsv.y;
  float v = hsv.z;
  
  float c = v * s;
  float x = c * (1.0 - abs(mod(h / 60.0, 2.0) - 1.0));
  float m = v - c;
  
  vec3 rgb = vec3(0.0);
  if (h >= 0.0 && h < 60.0) {
    rgb = vec3(c, x, 0.0);
  } else if (h >= 60.0 && h < 120.0) {
    rgb = vec3(x, c, 0.0);
  } else if (h >= 120.0 && h < 180.0) {
    rgb = vec3(0.0, c, x);
  } else if (h >= 180.0 && h < 240.0) {
    rgb = vec3(0.0, x, c);
  } else if (h >= 240.0 && h < 300.0) {
    rgb = vec3(x, 0.0, c);
  } else {
    rgb = vec3(c, 0.0, x);
  }
  
  return rgb + m;
}

float mandelBulb(vec3 pos) {
  vec3 z = pos;
  float dr = 1.0;
  float r = 0.0;
  
  for (int i = 0; i < iterations; i++) {
    r = length(z);
    if (r > bailout) {
      break;
    }
    
    // Convert to polar coordinates
    float theta = acos(z.z / r);
    float phi = atan(z.y, z.x);
    dr = pow(r, power - 1) * power * dr + 1;
    
    // Scale and rotate the point
    float zr = pow(r, power);
    theta = theta * power;
    phi = phi * power;
    
    // Convert back to cartesian coordinates
    z = zr * vec3(
      sin(theta) * cos(phi),
      sin(phi) * sin(theta),
      cos(theta)
    );
    
    z += pos;
  }
  
  return 0.5 * log(r) * r / dr;
}

float map(vec3 p) {
  return mandelBulb(p);
}

vec3 getNormal(vec3 p) {
  const vec2 eps = vec2(0.0001, 0.0);
  return normalize(vec3(
    map(p + eps.xyy) - map(p - eps.xyy),
    map(p + eps.yxy) - map(p - eps.yxy),
    map(p + eps.yyx) - map(p - eps.yyx)
  ));
}

vec3 rayMarching(vec3 rayOrigin, vec3 rayDirection) {
  float distanceTravelled = 0.0;
  
  for (int i = 0; i < maximalSteps; i++) {
    vec3 currentPos = rayOrigin + rayDirection * distanceTravelled;
    float dist = map(currentPos);
    
    if (dist < 0.001) {
      vec3 normal = getNormal(currentPos);
      float NdotL = clamp(dot(normal, lightDir), 0.0, 1.0);

      return vec3(0.2, 0.4, 0.8) * NdotL;
    }
    
    distanceTravelled += dist;
    
    if (distanceTravelled > maximalDistance) {
      // No intersection within the maximum distance
      break;
    }
  }
  
  // No intersection, return background color
  return vec3(0);
}

void main() {
  // Calculate ray
  vec3 worldUp = vec3(0, -1, 0);
  vec3 cameraRight = normalize(cross(worldUp, cameraDirection));
  vec3 cameraUp = normalize(cross(cameraDirection, cameraRight));
  vec2 ndc = tc * 2 - 1;
  ndc.x *= resolution.x / resolution.y;
  vec3 rayOrigin = cameraPosition;
  
  // Perform raymarching and compute color
  vec3 color = vec3(0);
  if (antiAliasing) {  
    for (int x = -1; x <= 1; ++x) {
      for (int y = -1; y <= 1; ++y) {
        vec2 offset = vec2(x, y) / resolution;
        vec3 rayDirection = cameraDirection + (ndc.x + offset.x) * cameraRight + (ndc.y + offset.y) * cameraUp;
        color += rayMarching(rayOrigin, normalize(rayDirection));
      }     
    }
    color /= 9;
  } else {
    vec3 rayDirection = normalize(cameraDirection + ndc.x * cameraRight + ndc.y * cameraUp);
    color = rayMarching(rayOrigin, rayDirection);
  }
  fragColor = vec4(color, 1);
}