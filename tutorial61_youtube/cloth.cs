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
    vec4 PositionIn [];
};
layout(std430, binding = 1) buffer PosOut
{
    vec4 PositionOut [];
};
layout(std430, binding = 2) buffer VelIn
{
    vec4 VelocityIn [];
};
layout(std430, binding = 3) buffer VelOut
{
    vec4 VelocityOut [];
};

void main()
{
    uvec3 nParticles = gl_NumWorkGroups * gl_WorkGroupSize;
    uint idx = gl_GlobalInvocationID.y * nParticles.x + gl_GlobalInvocationID.x;

    vec3 p = vec3(PositionIn[idx]);
    vec3 v = vec3(VelocityIn[idx]), r;

    // Start with gravitational acceleration and add the spring
    // forces from each neighbor
    vec3 force = Gravity * ParticleMass;

    // Particle above
    if (gl_GlobalInvocationID.y < nParticles.y - 1)
    {
        r = PositionIn[idx + nParticles.x].xyz - p;
        force += normalize(r) * SpringK * (length(r) - RestLengthVert);
    }
    // Below
    if (gl_GlobalInvocationID.y > 0)
    {
        r = PositionIn[idx - nParticles.x].xyz - p;
        force += normalize(r) * SpringK * (length(r) - RestLengthVert);
    }
    // Left
    if (gl_GlobalInvocationID.x > 0)
    {
        r = PositionIn[idx - 1].xyz - p;
        force += normalize(r) * SpringK * (length(r) - RestLengthHoriz);
    }
    // Right
    if (gl_GlobalInvocationID.x < nParticles.x - 1)
    {
        r = PositionIn[idx + 1].xyz - p;
        force += normalize(r) * SpringK * (length(r) - RestLengthHoriz);
    }

    // Diagonals
    // Upper-left
    if (gl_GlobalInvocationID.x > 0 && gl_GlobalInvocationID.y < nParticles.y - 1)
    {
        r = PositionIn[idx + nParticles.x - 1].xyz - p;
        force += normalize(r) * SpringK * (length(r) - RestLengthDiag);
    }
    // Upper-right
    if (gl_GlobalInvocationID.x < nParticles.x - 1 && gl_GlobalInvocationID.y < nParticles.y - 1)
    {
        r = PositionIn[idx + nParticles.x + 1].xyz - p;
        force += normalize(r) * SpringK * (length(r) - RestLengthDiag);
    }
    // lower -left
    if (gl_GlobalInvocationID.x > 0 && gl_GlobalInvocationID.y > 0)
    {
        r = PositionIn[idx - nParticles.x - 1].xyz - p;
        force += normalize(r) * SpringK * (length(r) - RestLengthDiag);
    }
    // lower-right
    if (gl_GlobalInvocationID.x < nParticles.x - 1 && gl_GlobalInvocationID.y > 0)
    {
        r = PositionIn[idx - nParticles.x + 1].xyz - p;
        force += normalize(r) * SpringK * (length(r) - RestLengthDiag);
    }

    force += -DampingConst * v;

    // Apply simple Euler integrator
    vec3 a = force * ParticleInvMass;
    PositionOut[idx] = vec4(
        p + v * DeltaT + 0.5 * a * DeltaT * DeltaT, 1.0);
    VelocityOut[idx] = vec4(v + a * DeltaT, 0.0);

    // Pin a few of the top verts
    if (gl_GlobalInvocationID.y == nParticles.y - 1 &&
        (gl_GlobalInvocationID.x == 0 ||
         gl_GlobalInvocationID.x == nParticles.x / 4 ||
         gl_GlobalInvocationID.x == nParticles.x * 2 / 4 ||
         gl_GlobalInvocationID.x == nParticles.x * 3 / 4 ||
         gl_GlobalInvocationID.x == nParticles.x - 1))
    {
        PositionOut[idx] = vec4(p, 1.0);
        VelocityOut[idx] = vec4(0, 0, 0, 0);
    }
}

