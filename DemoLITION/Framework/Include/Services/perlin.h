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


#pragma once

#include <vector>

struct PerlinConfig {
    int width = 256;
    int height = 256;
    float scale = 50.0f;
    int octaves = 4;
    float lacunarity = 2.0f;    
    float persistence = 0.5f;  
    int seed = 0;
};


void CreatePerlinMap(const PerlinConfig& Config, std::vector<float>& HeightMap);
