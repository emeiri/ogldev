/*
		Copyright 2026 Etay Meiri

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


#version 460

layout(local_size_x = 10, local_size_y = 10) in;

layout(binding = 0) uniform sampler2D HdrTex;

layout(std430, binding = 1) buffer Result {
    float TileLuminance[]; // One per workgroup
};

shared float localLum[gl_WorkGroupSize.x * gl_WorkGroupSize.y]; 

void main() 
{
    ivec2 texCoord = ivec2(gl_GlobalInvocationID.xy);
    vec3 hdr = texelFetch(HdrTex, texCoord, 0).rgb;

    float lum = dot(hdr, vec3(0.2126, 0.7152, 0.0722));

    localLum[gl_LocalInvocationIndex] = log(lum + 0.0001);
    
    barrier();

    // Thread 0 reduces tile luminance
    if (gl_LocalInvocationIndex == 0) {

        float Sum = 0.0;
        
        uint Size = gl_WorkGroupSize.x * gl_WorkGroupSize.y;
        
        for (uint i = 0; i < Size; ++i) {
            Sum += localLum[i];
        }

        TileLuminance[gl_WorkGroupID.y * gl_NumWorkGroups.x + gl_WorkGroupID.x] = Sum;
    }
}
