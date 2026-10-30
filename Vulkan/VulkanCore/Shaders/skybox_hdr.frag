/*

        Copyright 2024 Etay Meiri

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

layout (location=0) in vec3 Dir;

layout (location=0) out vec4 out_Color;

layout(set = 0, binding = 0) uniform UniformBuffer { mat4 VP; vec4 Config; } ubo;
layout(set = 0, binding = 1) uniform samplerCube cubeSampler;

// Narkowicz 2015, "ACES Filmic Tone Mapping Curve"
vec3 ToneMapACES(vec3 x)
{
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}


void main() 
{
    vec3 Color = textureLod(cubeSampler, Dir, 0.0).rgb * ubo.Config.x;

	out_Color = vec4(ToneMapACES(Color), 1.0);
};
