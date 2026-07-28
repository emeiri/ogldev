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

#include <vector>

#include "Services/terrain_grid.h"


void CreateTerrainGridVectors(int Width, int Height, std::vector<TerrainVertex>& Vertices, std::vector<u32>& Indices)
{
    float BaseCol = -Width / 2.0f;
    float BaseRow = -Height / 2.0f;

    // 1. Generate Vertex Buffer Data
    for (int row = 0; row < Height; ++row) {
        for (int col = 0; col < Width; ++col) {
            TerrainVertex vertex;

            vertex.position.x = BaseCol + (float)(col);
            vertex.position.y = BaseRow + (float)(row);

            // Normalize UV coordinates linearly from 0.0 to 1.0
            vertex.texCoords.x = (float)(col) / (float)(Width - 1);
            vertex.texCoords.y = (float)(row) / (float)(Height - 1);

            Vertices.push_back(vertex);
        }
    }

    // 2. Generate Index Buffer Data (Stitching quads together into triangles)
    for (int row = 0; row < Height - 1; ++row) {
        for (int col = 0; col < Width - 1; ++col) {
            // Find index pointers for the 4 corners of the current quad
            u32 topLeft = row * Width + col;
            u32 topRight = topLeft + 1;
            u32 bottomLeft = (row + 1) * Width + col;
            u32 bottomRight = bottomLeft + 1;

            // Triangle 1
            Indices.push_back(topLeft);
            Indices.push_back(bottomLeft);
            Indices.push_back(topRight);

            // Triangle 2
            Indices.push_back(topRight);
            Indices.push_back(bottomLeft);
            Indices.push_back(bottomRight);
        }
    }
}