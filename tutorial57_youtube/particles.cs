#version 430

layout( local_size_x = 1000 ) in;

uniform float Gravity1 = 1000.0;
uniform vec3 BlackHolePos1 = vec3(5,0,0);

uniform float Gravity2 = 1000.0;
uniform vec3 BlackHolePos2 = vec3(-5,0,0);

uniform float ParticleMass = 0.1;
uniform float ParticleInvMass = 1.0 / 0.1;
uniform float DeltaT = 0.0005;
uniform float MaxDist = 45.0;

layout(std430, binding=0) buffer Pos {
  vec4 Position[];
};
layout(std430, binding=1) buffer Vel {
  vec4 Velocity[];
};

void main() {
  uint idx = gl_GlobalInvocationID.x;

  vec3 p = Position[idx].xyz;

  // Force from black hole #1
  vec3 d = BlackHolePos1 - p;
  float dist = length(d);
  vec3 force = (Gravity1 / dist) * normalize(d);

  // Force from black hole #2
  d = BlackHolePos2 - p;
  dist = length(d);
  force += (Gravity2 / dist) * normalize(d);

  // Reset particles that get too far from the attractors
  if( dist > MaxDist ) {
    Position[idx] = vec4(0,0,0,1);
  } else {
    // Apply simple Euler integrator
    vec3 a = force * ParticleInvMass;
    Position[idx] = vec4(
        p + Velocity[idx].xyz * DeltaT + 0.5 * a * DeltaT * DeltaT, 1.0);
    Velocity[idx] = vec4( Velocity[idx].xyz + a * DeltaT, 0.0);
  }
}
