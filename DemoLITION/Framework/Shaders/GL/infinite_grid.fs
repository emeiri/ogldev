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

#version 330

in vec3 WorldPos;

layout(location = 0) out vec4 FragColor;

uniform float gGridCellSize = 0.025;
uniform vec4 gGridColorThin = vec4(0.5, 0.5, 0.5, 1.0);
uniform vec4 gGridColorThick = vec4(0.0, 0.0, 0.0, 1.0);




float satf(float x)
{
    float f = clamp(x, 0.0, 1.0);
    return f;
}


vec2 satv(vec2 x)
{
    vec2 v = clamp(x, vec2(0.0), vec2(1.0));
    return v;
}


float max2(vec2 v)
{
    float f = max(v.x, v.y);
    return f;
}


void main()
{
    vec2 dvx = vec2(dFdx(WorldPos.x), dFdy(WorldPos.x));
    vec2 dvy = vec2(dFdx(WorldPos.z), dFdy(WorldPos.z));

    float lx = length(dvx);
    float ly = length(dvy);

    vec2 dudv = vec2(lx, ly);

    dudv *= 4.0;

    // Step 1: use the Y derivative of y
    //float Lod0a = mod(WorldPos.z, gGridCellSize) / (4.0 * dFdy(WorldPos.z));

    // Step 2: use the X derivative of X
    //float Lod0a = mod(WorldPos.x, gGridCellSize) / (4.0 * dFdx(WorldPos.x));

    // Step 3: 
    //float Lod0a = mod(WorldPos.z, gGridCellSize) / (4.0 * ly);

    // Step 4:
    //float Lod0a = mod(WorldPos.x, gGridCellSize) / (4.0 * lx);

    // Step 5:
    //float Lod0a = max2(mod(WorldPos.xz, gGridCellSize) / dudv);

    // Step 6:
    //float Lod0a = max2(vec2(1.0) - mod(WorldPos.xz, gGridCellSize) / dudv);

    // Step 7:
    vec2 mod_div_dudv = mod(WorldPos.xz, gGridCellSize) / dudv;
    float Lod0a = max2(vec2(1.0) - abs(satv(mod_div_dudv) * 2.0 - vec2(1.0)) );

    vec4 Color = gGridColorThick;
    Color.a *= Lod0a;
   
    FragColor = Color;
}
