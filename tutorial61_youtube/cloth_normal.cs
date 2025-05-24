#version 430

layout( local_size_x = 10, local_size_y = 10 ) in;

layout(std430, binding=0) buffer PosIn {
  vec4 Position[];
};
layout(std430, binding=4) buffer NormOut {
  vec4 Normal[];
};

void main() {
  uvec3 nParticles = gl_NumWorkGroups * gl_WorkGroupSize;
  uint idx = gl_GlobalInvocationID.y * nParticles.x + gl_GlobalInvocationID.x;

  vec3 p = vec3(Position[idx]);
  vec3 n = vec3(0);
  vec3 a, b, c;

  if( gl_GlobalInvocationID.y < nParticles.y - 1) {
    c = Position[idx + nParticles.x].xyz - p;
    if( gl_GlobalInvocationID.x < nParticles.x - 1 ) {
      a = Position[idx + 1].xyz - p;
      b = Position[idx + nParticles.x + 1].xyz - p;
      n += cross(a,b);
      n += cross(b,c);
    }
    if( gl_GlobalInvocationID.x > 0 ) {
      a = c;
      b = Position[idx + nParticles.x - 1].xyz - p;
      c = Position[idx - 1].xyz - p;
      n += cross(a,b);
      n += cross(b,c);
    }
  }

  if( gl_GlobalInvocationID.y > 0 ) {
    c = Position[idx - nParticles.x].xyz - p;
    if( gl_GlobalInvocationID.x > 0 ) {
      a = Position[idx - 1].xyz - p;
      b = Position[idx - nParticles.x - 1].xyz - p;
      n += cross(a,b);
      n += cross(b,c);
    }
    if( gl_GlobalInvocationID.x < nParticles.x - 1 ) {
      a = c; 
      b = Position[idx - nParticles.x + 1].xyz - p;
      c = Position[idx + 1].xyz - p;
      n += cross(a,b);
      n += cross(b,c);
    }
  }

  Normal[idx] = vec4(normalize(n), 0.0);
}

