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


#version 450 core

in vec2 TexCoords;

layout(location = 0) out vec4 OutIndirect; // indirect diffuse (SSGI) output

layout(binding = 0) uniform sampler2D gAlbedoTex;   // G-Buffer albedo
layout(binding = 1) uniform sampler2D gNormalTex;   // G-Buffer normal
layout(binding = 2) uniform sampler2D gDepthTex;    // depth buffer

// Inverse projection matrix (of the projection used to render the G-Buffer)
uniform mat4 gInvProj;

uniform mat4 gView;
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

// Helper: decode normal from 0–1 back to -1..1
vec3 DecodeNormal(vec3 enc)
{
    return normalize(enc * 2.0 - 1.0);
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

    // DEBUG STAGE: For now, just visualize things instead of doing GI
    // We'll turn this into GI after we confirm reconstruction is correct.

    // Example: visualize linear depth
    float LinearDepth = Pview.z;  // view space: camera looks down -Z
    float DepthVis = LinearDepth / 200.0; // adjust 50.0 to your scene range
    DepthVis = clamp(DepthVis, 0.0, 1.0);

   // OutIndirect = vec4(1.0, 0.0, 0.0, 0.0);

  //  OutIndirect = vec4(vec3(Albedo), 1.0);
    //OutIndirect = vec4(vec3(Normal), 1.0);
   // OutIndirect = vec4(vec3(Depth), 1.0);      

    //OutIndirect = vec4(DepthVis);
    // R = depth, G = normal.y, B = normal.z
   // OutIndirect = vec4(DepthVis, Normal.y * 0.5 + 0.5, Normal.z * 0.5 + 0.5, 1.0);

   OutIndirect = vec4(Nview * 0.5 + 0.5, 1.0);
}