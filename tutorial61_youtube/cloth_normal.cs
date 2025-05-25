#version 430

layout( local_size_x = 10, local_size_y = 10 ) in;

layout(std430, binding=0) buffer PosIn {
  vec4 Position[];
};
layout(std430, binding=4) buffer NormOut {
  vec4 Normal[];
};

void main() {
  uvec3 NumParticles = gl_NumWorkGroups * gl_WorkGroupSize;
  uint Index = gl_GlobalInvocationID.y * NumParticles.x + gl_GlobalInvocationID.x;

  vec3 p = vec3(Position[Index]);
  vec3 n = vec3(0);
  vec3 a, b, c;

  if( gl_GlobalInvocationID.y < NumParticles.y - 1) {
    c = Position[Index + NumParticles.x].xyz - p;
    if( gl_GlobalInvocationID.x < NumParticles.x - 1 ) {
      a = Position[Index + 1].xyz - p;
      b = Position[Index + NumParticles.x + 1].xyz - p;
      n += cross(a,b);
      n += cross(b,c);
    }
    if( gl_GlobalInvocationID.x > 0 ) {
      a = c;
      b = Position[Index + NumParticles.x - 1].xyz - p;
      c = Position[Index - 1].xyz - p;
      n += cross(a,b);
      n += cross(b,c);
    }
  }

  if( gl_GlobalInvocationID.y > 0 ) {
    c = Position[Index - NumParticles.x].xyz - p;
    if( gl_GlobalInvocationID.x > 0 ) {
      a = Position[Index - 1].xyz - p;
      b = Position[Index - NumParticles.x - 1].xyz - p;
      n += cross(a,b);
      n += cross(b,c);
    }
    if( gl_GlobalInvocationID.x < NumParticles.x - 1 ) {
      a = c; 
      b = Position[Index - NumParticles.x + 1].xyz - p;
      c = Position[Index + 1].xyz - p;
      n += cross(a,b);
      n += cross(b,c);
    }
  }

  Normal[Index] = vec4(normalize(n), 0.0);
}

