#version 430

layout(local_size_x = 10, local_size_y = 10) in;

uniform vec3 Gravity = vec3(0,-10,0);
uniform float ParticleMass = 0.1;
uniform float ParticleInvMass = 1.0 / 0.1;
uniform float SpringK = 2000.0;
uniform float RestLengthHoriz;
uniform float RestLengthVert;
uniform float RestLengthDiag;
uniform float DeltaT = 0.000005;
uniform float DampingConst = 0.1;

layout(std430, binding = 0) buffer PosIn
{
    vec4 PositionIn[];
};

layout(std430, binding = 1) buffer PosOut
{
    vec4 PositionOut[];
};

layout(std430, binding = 2) buffer VelIn
{
    vec4 VelocityIn[];
};

layout(std430, binding = 3) buffer VelOut
{
    vec4 VelocityOut[];
};

void main()
{
    uvec3 NumParticles = gl_NumWorkGroups * gl_WorkGroupSize;
    uint Index = gl_GlobalInvocationID.y * NumParticles.x + gl_GlobalInvocationID.x;

    vec3 p = vec3(PositionIn[Index]);

    // Pin a few of the top verts
    if (gl_GlobalInvocationID.y == NumParticles.y - 1 &&
        (gl_GlobalInvocationID.x == 0 ||
         gl_GlobalInvocationID.x == NumParticles.x / 4 ||
         gl_GlobalInvocationID.x == NumParticles.x * 2 / 4 ||
         gl_GlobalInvocationID.x == NumParticles.x * 3 / 4 ||
         gl_GlobalInvocationID.x == NumParticles.x - 1))
    {      
        PositionOut[Index] = vec4(p, 1.0);
        return;
    }

    vec3 v; 

    // Start with gravitational acceleration and add the spring
    // Forces from each neighbor
    vec3 Force = Gravity * ParticleMass;

    // Particle above
    if (gl_GlobalInvocationID.y < NumParticles.y - 1) {
        v = PositionIn[Index + NumParticles.x].xyz - p;
        Force += normalize(v) * SpringK * (length(v) - RestLengthVert);
    }

    // Below
    if (gl_GlobalInvocationID.y > 0) {
        v = PositionIn[Index - NumParticles.x].xyz - p;
        Force += normalize(v) * SpringK * (length(v) - RestLengthVert);
    }

    // Left
    if (gl_GlobalInvocationID.x > 0) {
        v = PositionIn[Index - 1].xyz - p;
        Force += normalize(v) * SpringK * (length(v) - RestLengthHoriz);
    }

    // Right
    if (gl_GlobalInvocationID.x < NumParticles.x - 1) {
        v = PositionIn[Index + 1].xyz - p;
        Force += normalize(v) * SpringK * (length(v) - RestLengthHoriz);
    }

    // Diagonals
    // Upper-left
    if (gl_GlobalInvocationID.x > 0 && gl_GlobalInvocationID.y < NumParticles.y - 1) {
        v = PositionIn[Index + NumParticles.x - 1].xyz - p;
        Force += normalize(v) * SpringK * (length(v) - RestLengthDiag);
    }

    // Upper-right
    if (gl_GlobalInvocationID.x < NumParticles.x - 1 && gl_GlobalInvocationID.y < NumParticles.y - 1) {
        v = PositionIn[Index + NumParticles.x + 1].xyz - p;
        Force += normalize(v) * SpringK * (length(v) - RestLengthDiag);
    }

    // lower -left
    if (gl_GlobalInvocationID.x > 0 && gl_GlobalInvocationID.y > 0) {
        v = PositionIn[Index - NumParticles.x - 1].xyz - p;
        Force += normalize(v) * SpringK * (length(v) - RestLengthDiag);
    }

    // lower-right
    if (gl_GlobalInvocationID.x < NumParticles.x - 1 && gl_GlobalInvocationID.y > 0) {
        v = PositionIn[Index - NumParticles.x + 1].xyz - p;
        Force += normalize(v) * SpringK * (length(v) - RestLengthDiag);
    }

    vec3 Vel = vec3(VelocityIn[Index]);
    Force += -Vel * DampingConst;

    // Apply semi-implicit Euler integrator
    vec3 a = Force * ParticleInvMass;    
    VelocityOut[Index] = vec4(Vel + a * DeltaT, 0.0);

    Vel = vec3(VelocityIn[Index]);  // get the udpated value

    PositionOut[Index] = vec4(p + Vel * DeltaT + 0.5 * a * DeltaT * DeltaT, 1.0);
}

