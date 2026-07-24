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

#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>

#include "ogldev_util.h"
#include "Services/perlin.h"


// Step 1: Single-Layer Scaled Noise Pass
void CreatePerlinMap(const PerlinConfig& Config, std::vector<float>& HeightMap) 
{
    HeightMap.resize(Config.width * Config.height);
    
    for (int row = 0; row < Config.height; row++) {
        for (int col = 0; col < Config.width; col++) {
            float SampleX = col / Config.scale;
            float SampleY = row / Config.scale;

            float p = glm::perlin(glm::vec2(SampleX, SampleY));

            HeightMap[row * Config.width + col] = p;
        }
    }
}
