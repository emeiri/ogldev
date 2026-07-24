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

    std::vector<glm::vec2> octaveOffsets(Config.octaves);
    srand(Config.seed);
    
    for (int i = 0; i < Config.octaves; i++) {
        float offsetX = (float)(rand() % 200000 - 100000);
        float offsetY = (float)(rand() % 200000 - 100000);
        octaveOffsets[i] = glm::vec2(offsetX, offsetY);
    }

    float MinValue = std::numeric_limits<float>::max();
    float MaxValue = std::numeric_limits<float>::lowest();

    for (int row = 0; row < Config.height; row++) {
        for (int col = 0; col < Config.width; col++) {

            float Sum = 0.0f;
            float Amplitude = 1.0f;
            float Freq = 1.0f;

            for (int Oct = 0; Oct < Config.octaves; Oct++) {
                float SampleX = col / Config.scale * Freq + octaveOffsets[Oct].x;
                float SampleY = row / Config.scale * Freq + octaveOffsets[Oct].y;

                float p = glm::perlin(glm::vec2(SampleX, SampleY)) * Amplitude;
               
                Sum += p;

                Freq *= Config.lacunarity;
                Amplitude *= Config.persistence;
            }

            HeightMap[row * Config.width + col] = Sum;

            MinValue = std::min(MinValue, Sum);
            MaxValue = std::max(MaxValue, Sum);
        }
    }

    for (int i = 0; i < HeightMap.size(); ++i) {
        if (MaxValue != MinValue) {
            float NormalizedHeight = (HeightMap[i] - MinValue) / (MaxValue - MinValue);

            HeightMap[i] = NormalizedHeight;
        } else {
            HeightMap[i] = 0.0f;
        }
    }
}
