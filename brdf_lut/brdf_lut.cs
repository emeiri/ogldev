// based on https://github.com/SaschaWillems/Vulkan-glTF-PBR/blob/master/data/shaders/genbrdflut.frag
#version 460
#extension GL_KHR_vulkan_glsl : enable

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout (constant_id = 0) const uint NUM_SAMPLES = 1024u;

layout (set = 0, binding = 0) buffer SRC { float data[]; } src;
layout (set = 0, binding = 1) buffer DST { float data[]; } dst;

const uint BRDF_W = 256;
const uint BRDF_H = 256;

const float PI = 3.1415926536;

// based on http://byteblacksmith.com/improvements-to-the-canonical-one-liner-glsl-rand-for-opengl-es-2-0/
float random(vec2 co)
{
	float a = 12.9898;
	float b = 78.233;
	float c = 43758.5453;
	float dt= dot(co.xy ,vec2(a,b));
	float sn= mod(dt,3.14);
	return fract(sin(sn) * c);
}

vec2 hammersley2d(uint i, uint N) 
{
	// Radical inverse based on http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
	uint bits = (i << 16u) | (i >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	float rdi = float(bits) * 2.3283064365386963e-10;
	return vec2(float(i) /float(N), rdi);
}

// based on http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_slides.pdf
vec3 importanceSample_GGX(vec2 Xi, float roughness, vec3 normal) 
{
	// Maps a 2D point to a hemisphere with spread based on roughness
	float alpha = roughness * roughness;
	float phi = 2.0 * PI * Xi.x + random(normal.xz) * 0.1;
	float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (alpha*alpha - 1.0) * Xi.y));
	float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
	vec3 H = vec3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);

	// Tangent space
	vec3 up = abs(normal.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 tangentX = normalize(cross(up, normal));
	vec3 tangentY = normalize(cross(normal, tangentX));

	// Convert to world Space
	return normalize(tangentX * H.x + tangentY * H.y + normal * H.z);
}

// Geometric Shadowing function
float G_SchlicksmithGGX(float dotNL, float dotNV, float roughness)
{
	float k = (roughness * roughness) / 2.0;
	float GL = dotNL / (dotNL * (1.0 - k) + k);
	float GV = dotNV / (dotNV * (1.0 - k) + k);
	return GL * GV;
}

vec2 BRDF(float NoV, float roughness)
{
	// Normal always points along z-axis for the 2D lookup 
	const vec3 N = vec3(0.0, 0.0, 1.0);
	vec3 V = vec3(sqrt(1.0 - NoV*NoV), 0.0, NoV);

	vec2 LUT = vec2(0.0);
	for(uint i = 0u; i < NUM_SAMPLES; i++)
	{
       // vec2 Xi = vec2((float)i / (float)NUM_SAMPLES, (float)i / (float)NUM_SAMPLES);
        vec2 Xi = hammersley2d(i, NUM_SAMPLES);
        vec3 H = importanceSample_GGX(Xi, roughness, N);
		vec3 L = 2.0 * dot(V, H) * H - V;

		float dotNL = max(dot(N, L), 0.0);

		if (dotNL > 0.0) {
            float dotNV = max(dot(N, V), 0.0);
            float dotVH = max(dot(V, H), 0.0);
            float dotNH = max(dot(H, N), 0.0);

            float G = G_SchlicksmithGGX(dotNL, dotNV, roughness);
			float G_Vis = (G * dotVH) / (dotNH * dotNV);
			float Fc = pow(1.0 - dotVH, 5.0);
			LUT.x += (1.0 - Fc) * G_Vis;
			LUT.y += Fc * G_Vis;
		}
	}

	return LUT / float(NUM_SAMPLES);
}

void main() 
{
	vec2 uv;
	uv.x = (float(gl_GlobalInvocationID.x) + 0.5) / float(BRDF_W);
	uv.y = (float(gl_GlobalInvocationID.y) + 0.5) / float(BRDF_H);

	vec2 v = BRDF(uv.x, 1.0 - uv.y);

	uint offset = gl_GlobalInvocationID.y * BRDF_W + gl_GlobalInvocationID.x;

	dst.data[offset * 2 + 0] = v.x;
	dst.data[offset * 2 + 1] = v.y;
}
