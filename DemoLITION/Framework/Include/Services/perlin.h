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
    int width = 257;        // Power-of-two plus one (ideal for tessellation patches)
    int height = 257;
    int octaves = 5;         // How many detail layers to stack
    float lacunarity = 2.0f; // Frequency multiplier per octave (keep near 2.0)
    float persistence = 0.5f;  // Amplitude multiplier per octave
    float scale = 2.0f;    // Initial zoom scale for the coordinate space
    float horizontalScale = 4.5f; // Horizontal scaling factor for the terrain
    float maxHeight = 100; // Maximum height of the terrain in world units
};


void CreatePerlinMap(const PerlinConfig& Config, std::vector<float>& HeightMap);
