/*

        Copyright 2025 Etay Meiri

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#version 460 core

in vec2 TexCoords;

layout(location = 0) out vec4 OutIndirect; // indirect diffuse (SSGI) output

layout(binding = 0) uniform sampler2D gAlbedoTex;   // G-Buffer albedo
layout(binding = 1) uniform sampler2D gNormalTex;   // G-Buffer normal
layout(binding = 2) uniform sampler2D gDepthTex;    // depth buffer


uniform mat4 gProj;
uniform mat4 gInvProj;  // Inverse projection matrix (of the projection used to render the G-Buffer)
uniform mat4 gView;
uniform vec2 gScreenSize = vec2(1920, 1080);
// If you want world-space later, add uInvView as well

// Helper: reconstruct view-space position from depth + UV
vec3 ReconstructViewPos(vec2 TexCoords, float depth)
{
    // depth is in [0,1] from the depth buffer, convert to NDC z in [-1,1]
    float z = depth * 2.0 - 1.0;

    // NDC position
    vec4 ndc = vec4(TexCoords * 2.0 - 1.0, z, 1.0);

    // Back to view space
    vec4 view = gInvProj * ndc;
    view /= view.w;

    return view.xyz; // view-space position
}


mat3 buildTangentBasis(vec3 N)
{
    vec3 up = (abs(N.z) < 0.999) ? vec3(0, 0, 1) : vec3(1, 0, 0);
    vec3 T = normalize(cross(up, N));
    vec3 B = cross(N, T);
    return mat3(T, B, N); // columns: T, B, N
}


float hash(vec2 p) {
    return fract(sin(dot(p, vec2(12.9898,78.233))) * 43758.5453);
}

vec3 sampleHemisphereSimple(vec2 uv)
{
    float r1 = hash(uv);
    float r2 = hash(uv.yx + 13.37);

    float phi = r1 * 6.2831853; // 2 * PI
    float cosTheta = r2;
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    vec3 local = vec3(cos(phi) * sinTheta,
                      sin(phi) * sinTheta,
                      cosTheta); // hemisphere around +Z
    return local;
}


uint hash_u(uint x)
{
    x ^= x >> 16;
    x *= 0x7feb352dU;
    x ^= x >> 15;
    x *= 0x846ca68bU;
    x ^= x >> 16;
    return x;
}

float rand_u32(uint x)
{
    return float(hash_u(x)) / 4294967295.0;
}

vec3 sampleHemisphere(vec2 uv, int rayIndex)
{
    // Convert to integer pixel coords
    ivec2 pixel = ivec2(uv * gScreenSize);

    uint seed1 = uint(pixel.x) ^ (uint(pixel.y) << 16) ^ uint(rayIndex * 977);
    uint seed2 = uint(pixel.y) ^ (uint(pixel.x) << 16) ^ uint(rayIndex * 1319);

    float r1 = rand_u32(seed1);
    float r2 = rand_u32(seed2);

    float phi = r1 * 6.2831853; // 

    // Better: cosine-weighted sampling
    float cosTheta = sqrt(1.0 - r2);
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    return vec3(cos(phi) * sinTheta,
                sin(phi) * sinTheta,
                cosTheta);
}

void rayMarch(
    vec3 Pview,
    vec3 rayDir,
    out bool hit,
    out vec2 hitUV,
    out float traveled)
{
    hit = false;
    traveled = 0.0;

    const float STEP_SIZE = 0.15;
    const float THICKNESS = 0.75;
    const float MAX_DISTANCE = 20.0;
    const int   NUM_STEPS = 32;

    vec3 marchPos = Pview;

    for (int i = 0; i < NUM_STEPS; i++)
    {
        marchPos += rayDir * STEP_SIZE;
        traveled += STEP_SIZE;

        if (traveled > MAX_DISTANCE)
            return;

        // project to screen
        vec4 clip = gProj * vec4(marchPos, 1.0);
        vec3 ndc  = clip.xyz / clip.w;
        vec2 suv  = ndc.xy * 0.5 + 0.5;

        if (suv.x < 0.0 || suv.x > 1.0 || suv.y < 0.0 || suv.y > 1.0)
            return;

        float sceneDepth = texture(gDepthTex, suv).r;
        if (sceneDepth == 1.0)
            continue;

        vec3 scenePos = ReconstructViewPos(suv, sceneDepth);

        // left-handed: +Z forward
        float dz = marchPos.z - scenePos.z;

        if (abs(dz) < THICKNESS)
        {
            hit = true;
            hitUV = suv;
            return;
        }
    }
}

void main()
{
    // 1) Fetch G-Buffer data
    float Depth = texture(gDepthTex, TexCoords).r;

    // Early out if depth is 1.0 (no geometry; far plane / sky)
    if (Depth == 1.0) {
        OutIndirect = vec4(0.0);
        return;
    }

    vec3 Albedo = texture(gAlbedoTex, TexCoords).rgb;
    vec3 Nworld = normalize(texture(gNormalTex, TexCoords).rgb);
    mat3 View3x3 = transpose(inverse(mat3(gView)));     // TODO: move to C++
    vec3 Nview = normalize(View3x3 * Nworld);

    // 2) Reconstruct view-space position
    vec3 Pview = ReconstructViewPos(TexCoords, Depth);

    mat3 TBN = buildTangentBasis(Nview);

    vec3 indirect = vec3(0.0);

   const int NUM_RAYS = 8;
float occlusion = 0.0;

for (int r = 0; r < NUM_RAYS; r++)
{
    vec3 dirLocal = sampleHemisphere(TexCoords, r);
    vec3 rayDir   = normalize(TBN * dirLocal);

    // start slightly above the surface to avoid self-hit
    vec3 startPos = Pview + Nview * 0.05;

    bool hit;
    vec2 hitUV;
    float traveled;

    rayMarch(startPos, rayDir, hit, hitUV, traveled);

    if (hit)
    {
        // weight close hits more, distant hits less
        float w = 1.0 / (1.0 + traveled * 0.5);
        occlusion += w;
    }
}

// normalize and remap
occlusion /= float(NUM_RAYS);

// remap so “fully occluded” isn’t black but maybe 0.3
float minAO = 0.3;
float ao = mix(1.0, minAO, clamp(occlusion, 0.0, 1.0));

OutIndirect = vec4(vec3(ao), 1.0);

    //---------------------------------------------------------------------

    // DEBUG STAGE: For now, just visualize things instead of doing GI
    // We'll turn this into GI after we confirm reconstruction is correct.

    // Example: visualize linear depth
  //  float LinearDepth = Pview.z;  // view space: camera looks down -Z
 //   float DepthVis = LinearDepth / 200.0; // adjust 50.0 to your scene range
 //   DepthVis = clamp(DepthVis, 0.0, 1.0);

   // OutIndirect = vec4(1.0, 0.0, 0.0, 0.0);

  //  OutIndirect = vec4(vec3(Albedo), 1.0);
    //OutIndirect = vec4(vec3(Normal), 1.0);
   // OutIndirect = vec4(vec3(Depth), 1.0);      

    //OutIndirect = vec4(DepthVis);
    // R = depth, G = normal.y, B = normal.z
   // OutIndirect = vec4(DepthVis, Normal.y * 0.5 + 0.5, Normal.z * 0.5 + 0.5, 1.0);

  // OutIndirect = vec4(Nview, 1.0);

  //OutIndirect = vec4(rayDir, 1.0);

  //if (hit)
    //OutIndirect = vec4(1.0, 0.0, 0.0, 1.0);  // red = hit
//else
    //OutIndirect = vec4(0.0, 0.0, 0.0, 1.0);  // black = miss

   // float distVis = traveled / MAX_DISTANCE;
   // OutIndirect = vec4(distVis, distVis, distVis, 1.0);

   //vec3 indirect = vec3(0.0);

   // if (hit)
   // {
        // 1. Sample the albedo at the hit point
     //   vec3 hitAlbedo = texture(gAlbedoTex, hitUV).rgb;

        // 2. Cosine weighting (Lambertian BRDF)
     //  float nDotL = max(dot(Nview, rayDir), 0.0);

        // 3. Final indirect contribution for this ray
      //  indirect = hitAlbedo * nDotL;
  //  }

   // OutIndirect = vec4(indirect, 1.0);
}