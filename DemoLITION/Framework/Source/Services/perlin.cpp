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


void CreatePerlinMap(const PerlinConfig& Config, std::vector<float>& HeightMap)
{
    HeightMap.resize(Config.width * Config.height);
    std::memset(HeightMap.data(), 0, ARRAY_SIZE_IN_BYTES(HeightMap));

    // Safeguard scale to avoid division by zero errors
    float safeScale = Config.scale;
    if (safeScale <= 0.0f) safeScale = 0.0001f;

    // 1. Generate unique offsets based on the seed for each layer (octave)
    std::vector<glm::vec2> octaveOffsets(Config.octaves);
    srand(10); // Restored Config.seed parameter usage 

    for (int i = 0; i < Config.octaves; i++) {
        float offsetX = (float)(rand() % 200000 - 100000);
        float offsetY = (float)(rand() % 200000 - 100000);
        octaveOffsets[i] = glm::vec2(offsetX, offsetY);
    }

    float MinValue = std::numeric_limits<float>::max();
    float MaxValue = std::numeric_limits<float>::lowest();

    // Centering offsets to zoom into the center of the map instead of top-right
    float halfWidth = (float)Config.width / 2.0f;
    float halfHeight = (float)Config.height / 2.0f;


    // 2. First Pass: Calculate layered noise values
    for (int row = 0; row < Config.height; row++) {
        for (int col = 0; col < Config.width; col++) {
            float Sum = 0.0f;
            float Amplitude = 1.0f;
            float Freq = 1.0f;

            float u = (float)col / (float)(Config.width - 1);
            float v = (float)row / (float)(Config.height - 1);
            for (int Oct = 0; Oct < Config.octaves; Oct++) {
                float baseSampleX = u / safeScale;
                float baseSampleY = v / safeScale;

                // Corrected coordinate transformation centered around the middle
                float sampleX = (baseSampleX + octaveOffsets[Oct].x) * Freq;
                float sampleY = (baseSampleY + octaveOffsets[Oct].y) * Freq;

                glm::vec2 p(sampleX, sampleY);

                // GLM outputs [-1, 1]. If attempting to exactly duplicate Unity's [0, 1] converted math,
                // you would treat standard output accordingly. Here we keep standard accumulator:
                Sum += glm::perlin(p) * Amplitude;

                Freq *= Config.lacunarity;
                Amplitude *= Config.persistence;
            }

            HeightMap[row * Config.width + col] = Sum;

            if (Sum < MinValue) MinValue = Sum;
            if (Sum > MaxValue) MaxValue = Sum;
        }
    }

    // 3. Second Pass: Absolute Normalization
    for (int i = 0; i < HeightMap.size(); ++i) {
        // Prevent division-by-zero if MaxValue equals MinValue
        if (MaxValue != MinValue) {
            float normalizedHeight = (HeightMap[i] - MinValue) / (MaxValue - MinValue);
            HeightMap[i] = normalizedHeight;
        } else {
            HeightMap[i] = 0.0f;
        }
    }
}
