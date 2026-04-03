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

#include "GL/gl_blur_filter1_technique.h"

#define SIGMA_V 25.0f


float Gauss(float x, float sigma)
{
    double coeff = 1.0 / (glm::two_pi<double>() * sigma);
    double expon = -(x * x) / (2.0 * sigma);
    return (float)(coeff * exp(expon));
}


bool BlurFilter1Technique::Init()
{
    if (!FullScreenTechnique::Init("Framework/Shaders/GL/blur_filter1.fs")) {
        return false;
    }

    GET_UNIFORM(gSampler);

    GetWeightsLocations();

    Enable();

    glUniform1i(m_gSamplerLoc, 0);

    SetSigmaV(SIGMA_V);

    return true;
}


void BlurFilter1Technique::GetWeightsLocations()
{
    for (int i = 0; i < ARRAY_SIZE_IN_ELEMENTS(m_weights); i++) {
        char Name[128] = { 0 };
        SNPRINTF(Name, sizeof(Name), "Weights[%d]", i);

        m_weights[i] = GetUniformLocation(Name);

        if (m_weights[i] == (GLuint)-1) {
            printf("Error getting uniform location for weight %d\n", i);
            exit(1);
        }
    }
}


void BlurFilter1Technique::SetSigmaV(float Sigma)
{
    float Weights[NUM_WEIGHTS] = { 0.0f }, Sum = 0.0f;

    // Compute and sum the weights
    Weights[0] = Gauss(0, Sigma);
    Sum = Weights[0];

    for (int i = 1; i < NUM_WEIGHTS; i++) {
        Weights[i] = Gauss(float(i), Sigma);
        Sum += 2 * Weights[i];
    }

    // Normalize the weights and set the uniform
    for (int i = 0; i < NUM_WEIGHTS; i++) {
        float Val = Weights[i] / Sum;
        SetWeight(i, Val);
    }
}


void BlurFilter1Technique::SetWeight(uint Index, float Weight)
{
    if (Index >= NUM_WEIGHTS) {
        printf("Invalid weight index %d\n", Index);
        exit(1);
    }

    glUniform1f(m_weights[Index], Weight);
}
