/*

	Copyright 2011 Etay Meiri

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

#include <limits.h>
#include <string.h>

#include "math_3d.h"
#include "lighting_technique.h"
#include "util.h"

static const char* pVS = "                                                                      \n\
#version 410 core                                                                               \n\
                                                                                                \n\
layout (location = 0) in vec3 Position_VS_in;                                                   \n\
layout (location = 1) in vec2 TexCoord_VS_in;                                                   \n\
layout (location = 2) in vec3 Normal_VS_in;                                                     \n\
                                                                                                \n\
uniform mat4 gWorld;                                                                            \n\
                                                                                                \n\
out vec3 WorldPos_CS_in;                                                                        \n\
out vec2 TexCoord_CS_in;                                                                        \n\
out vec3 Normal_CS_in;                                                                          \n\
                                                                                                \n\
void main()                                                                                     \n\
{                                                                                               \n\
    WorldPos_CS_in = (gWorld * vec4(Position_VS_in, 1.0)).xyz;                                  \n\
    TexCoord_CS_in = TexCoord_VS_in;                                                            \n\
    Normal_CS_in   = (gWorld * vec4(Normal_VS_in, 0.0)).xyz;                                    \n\
}";


static const char* pTessCS = "                                                                  \n\
#version 410 core                                                                               \n\
                                                                                                \n\
// define the number of CPs in the output patch                                                 \n\
layout (vertices = 3) out;                                                                      \n\
                                                                                                \n\
uniform vec3 gEyeWorldPos;                                                                      \n\
                                                                                                \n\
// attributes of the input CPs                                                                  \n\
in vec3 WorldPos_CS_in[];                                                                       \n\
in vec2 TexCoord_CS_in[];                                                                       \n\
in vec3 Normal_CS_in[];                                                                         \n\
                                                                                                \n\
// attributes of the output CPs                                                                 \n\
out vec3 WorldPos_ES_in[];                                                                      \n\
out vec2 TexCoord_ES_in[];                                                                      \n\
out vec3 Normal_ES_in[];                                                                        \n\
                                                                                                \n\
float GetTessLevel(float Distance0, float Distance1)                                            \n\
{                                                                                               \n\
    float AvgDistance = (Distance0 + Distance1) / 2.0;                                          \n\
                                                                                                \n\
    if (AvgDistance <= 2.0) {                                                                   \n\
        return 10.0;                                                                            \n\
    }                                                                                           \n\
    else if (AvgDistance <= 5.0) {                                                              \n\
        return 7.0;                                                                             \n\
    }                                                                                           \n\
    else {                                                                                      \n\
        return 3.0;                                                                             \n\
    }                                                                                           \n\
}                                                                                               \n\
                                                                                                \n\
void main()                                                                                     \n\
{                                                                                               \n\
    // Set the control points of the output patch                                               \n\
    TexCoord_ES_in[gl_InvocationID] = TexCoord_CS_in[gl_InvocationID];                          \n\
    Normal_ES_in[gl_InvocationID]   = Normal_CS_in[gl_InvocationID];                            \n\
    WorldPos_ES_in[gl_InvocationID] = WorldPos_CS_in[gl_InvocationID];                          \n\
                                                                                                \n\
    // Calculate the distance from the camera to the three control points                       \n\
    float EyeToVertexDistance0 = distance(gEyeWorldPos, WorldPos_ES_in[0]);                     \n\
    float EyeToVertexDistance1 = distance(gEyeWorldPos, WorldPos_ES_in[1]);                     \n\
    float EyeToVertexDistance2 = distance(gEyeWorldPos, WorldPos_ES_in[2]);                     \n\
                                                                                                \n\
    // Calculate the tessellation levels                                                        \n\
    gl_TessLevelOuter[0] = GetTessLevel(EyeToVertexDistance1, EyeToVertexDistance2);            \n\
    gl_TessLevelOuter[1] = GetTessLevel(EyeToVertexDistance2, EyeToVertexDistance0);            \n\
    gl_TessLevelOuter[2] = GetTessLevel(EyeToVertexDistance0, EyeToVertexDistance1);            \n\
    gl_TessLevelInner[0] = gl_TessLevelOuter[2];                                                \n\
}                                                                                               \n\
";


static const char* pTessES = "                                                                  \n\
#version 410 core                                                                               \n\
                                                                                                \n\
layout(triangles, equal_spacing, ccw) in;                                                       \n\
                                                                                                \n\
uniform mat4 gVP;                                                                               \n\
uniform sampler2D gDisplacementMap;                                                             \n\
uniform float gDispFactor;                                                                      \n\
                                                                                                \n\
in vec3 WorldPos_ES_in[];                                                                       \n\
in vec2 TexCoord_ES_in[];                                                                       \n\
in vec3 Normal_ES_in[];                                                                         \n\
                                                                                                \n\
out vec3 WorldPos_FS_in;                                                                        \n\
out vec2 TexCoord_FS_in;                                                                        \n\
out vec3 Normal_FS_in;                                                                          \n\
                                                                                                \n\
vec2 interpolate2D(vec2 v0, vec2 v1, vec2 v2)                                                   \n\
{                                                                                               \n\
    return vec2(gl_TessCoord.x) * v0 + vec2(gl_TessCoord.y) * v1 + vec2(gl_TessCoord.z) * v2;   \n\
}                                                                                               \n\
                                                                                                \n\
vec3 interpolate3D(vec3 v0, vec3 v1, vec3 v2)                                                   \n\
{                                                                                               \n\
    return vec3(gl_TessCoord.x) * v0 + vec3(gl_TessCoord.y) * v1 + vec3(gl_TessCoord.z) * v2;   \n\
}                                                                                               \n\
                                                                                                \n\
void main()                                                                                     \n\
{                                                                                               \n\
    // Interpolate the attributes of the output vertex using the barycentric coordinates        \n\
    TexCoord_FS_in = interpolate2D(TexCoord_ES_in[0], TexCoord_ES_in[1], TexCoord_ES_in[2]);    \n\
    Normal_FS_in = interpolate3D(Normal_ES_in[0], Normal_ES_in[1], Normal_ES_in[2]);            \n\
    Normal_FS_in = normalize(Normal_FS_in);                                                     \n\
    WorldPos_FS_in = interpolate3D(WorldPos_ES_in[0], WorldPos_ES_in[1], WorldPos_ES_in[2]);    \n\
                                                                                                \n\
    // Displace the vertex along the normal                                                     \n\
    float Displacement = texture(gDisplacementMap, TexCoord_FS_in.xy).x;                        \n\
    WorldPos_FS_in += Normal_FS_in * Displacement * gDispFactor;                                \n\
    gl_Position = gVP * vec4(WorldPos_FS_in, 1.0);                                              \n\
}                                                                                               \n\
";

static const char* pFS = "                                                                  \n\
#version 410 core                                                                           \n\
                                                                                            \n\
const int MAX_POINT_LIGHTS = 2;                                                             \n\
const int MAX_SPOT_LIGHTS = 2;                                                              \n\
                                                                                            \n\
in vec2 TexCoord_FS_in;                                                                     \n\
in vec3 Normal_FS_in;                                                                       \n\
in vec3 WorldPos_FS_in;                                                                     \n\
                                                                                            \n\
out vec4 FragColor;                                                                         \n\
                                                                                            \n\
struct BaseLight                                                                            \n\
{                                                                                           \n\
    vec3 Color;                                                                             \n\
    float AmbientIntensity;                                                                 \n\
    float DiffuseIntensity;                                                                 \n\
};                                                                                          \n\
                                                                                            \n\
struct DirectionalLight                                                                     \n\
{                                                                                           \n\
    BaseLight Base;                                                                  \n\
    vec3 Direction;                                                                         \n\
};                                                                                          \n\
                                                                                            \n\
struct Attenuation                                                                          \n\
{                                                                                           \n\
    float Constant;                                                                         \n\
    float Linear;                                                                           \n\
    float Exp;                                                                              \n\
};                                                                                          \n\
                                                                                            \n\
struct PointLight                                                                           \n\
{                                                                                           \n\
    BaseLight Base;                                                                  \n\
    vec3 Position;                                                                          \n\
    Attenuation Atten;                                                                      \n\
};                                                                                          \n\
                                                                                            \n\
struct SpotLight                                                                            \n\
{                                                                                           \n\
    struct PointLight Base;                                                                 \n\
    vec3 Direction;                                                                         \n\
    float Cutoff;                                                                           \n\
};                                                                                          \n\
                                                                                            \n\
uniform int gNumPointLights;                                                                \n\
uniform int gNumSpotLights;                                                                 \n\
uniform DirectionalLight gDirectionalLight;                                                 \n\
uniform PointLight gPointLights[MAX_POINT_LIGHTS];                                          \n\
uniform SpotLight gSpotLights[MAX_SPOT_LIGHTS];                                             \n\
uniform sampler2D gColorMap;                                                                \n\
uniform vec3 gEyeWorldPos;                                                                  \n\
uniform float gMatSpecularIntensity;                                                        \n\
uniform float gSpecularPower;                                                               \n\
                                                                                            \n\
vec4 CalcLightInternal(BaseLight Light, vec3 LightDirection, vec3 Normal)            \n\
{                                                                                           \n\
    vec4 AmbientColor = vec4(Light.Color, 1.0f) * Light.AmbientIntensity;                   \n\
    float DiffuseFactor = dot(Normal, -LightDirection);                                     \n\
                                                                                            \n\
    vec4 DiffuseColor  = vec4(0, 0, 0, 0);                                                  \n\
    vec4 SpecularColor = vec4(0, 0, 0, 0);                                                  \n\
                                                                                            \n\
    if (DiffuseFactor > 0) {                                                                \n\
        DiffuseColor = vec4(Light.Color, 1.0f) * Light.DiffuseIntensity * DiffuseFactor;    \n\
                                                                                            \n\
        vec3 VertexToEye = normalize(gEyeWorldPos - WorldPos_FS_in);                        \n\
        vec3 LightReflect = normalize(reflect(LightDirection, Normal));                     \n\
        float SpecularFactor = dot(VertexToEye, LightReflect);                              \n\
        SpecularFactor = pow(SpecularFactor, gSpecularPower);                               \n\
        if (SpecularFactor > 0) {                                                           \n\
            SpecularColor = vec4(Light.Color, 1.0f) *                                       \n\
                            gMatSpecularIntensity * SpecularFactor;                         \n\
        }                                                                                   \n\
    }                                                                                       \n\
                                                                                            \n\
    return (AmbientColor + DiffuseColor + SpecularColor);                                   \n\
}                                                                                           \n\
                                                                                            \n\
vec4 CalcDirectionalLight(vec3 Normal)                                                      \n\
{                                                                                           \n\
    return CalcLightInternal(gDirectionalLight.Base, gDirectionalLight.Direction, Normal);  \n\
}                                                                                           \n\
                                                                                            \n\
vec4 CalcPointLight(struct PointLight l, vec3 Normal)                                       \n\
{                                                                                           \n\
    vec3 LightDirection = WorldPos_FS_in - l.Position;                                      \n\
    float Distance = length(LightDirection);                                                \n\
    LightDirection = normalize(LightDirection);                                             \n\
                                                                                            \n\
    vec4 Color = CalcLightInternal(l.Base, LightDirection, Normal);                         \n\
    float Attenuation =  l.Atten.Constant +                                                 \n\
                         l.Atten.Linear * Distance +                                        \n\
                         l.Atten.Exp * Distance * Distance;                                 \n\
                                                                                            \n\
    return Color / Attenuation;                                                             \n\
}                                                                                           \n\
                                                                                            \n\
vec4 CalcSpotLight(struct SpotLight l, vec3 Normal)                                         \n\
{                                                                                           \n\
    vec3 LightToPixel = normalize(WorldPos_FS_in - l.Base.Position);                        \n\
    float SpotFactor = dot(LightToPixel, l.Direction);                                      \n\
                                                                                            \n\
    if (SpotFactor > l.Cutoff) {                                                            \n\
        vec4 Color = CalcPointLight(l.Base, Normal);                                        \n\
        return Color * (1.0 - (1.0 - SpotFactor) * 1.0/(1.0 - l.Cutoff));                   \n\
    }                                                                                       \n\
    else {                                                                                  \n\
        return vec4(0,0,0,0);                                                               \n\
    }                                                                                       \n\
}                                                                                           \n\
                                                                                            \n\
void main()                                                                                 \n\
{                                                                                           \n\
    vec3 Normal = normalize(Normal_FS_in);                                                  \n\
    vec4 TotalLight = CalcDirectionalLight(Normal);                                         \n\
                                                                                            \n\
    for (int i = 0 ; i < gNumPointLights ; i++) {                                           \n\
        TotalLight += CalcPointLight(gPointLights[i], Normal);                              \n\
    }                                                                                       \n\
                                                                                            \n\
    for (int i = 0 ; i < gNumSpotLights ; i++) {                                            \n\
        TotalLight += CalcSpotLight(gSpotLights[i], Normal);                                \n\
    }                                                                                       \n\
                                                                                            \n\
    FragColor = texture(gColorMap, TexCoord_FS_in.xy) * TotalLight;                         \n\
}";



LightingTechnique::LightingTechnique()
{   
}

bool LightingTechnique::Init()
{
    if (!Technique::Init()) {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, pVS)) {
        return false;
    }

    if (!AddShader(GL_TESS_CONTROL_SHADER, pTessCS)) {
        return false;
    }

    if (!AddShader(GL_TESS_EVALUATION_SHADER, pTessES)) {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, pFS)) {
        return false;
    }
    
    if (!Finalize()) {
        return false;
    }

    m_VPLocation = GetUniformLocation("gVP");
    m_WorldMatrixLocation = GetUniformLocation("gWorld");
    m_colorTextureLocation = GetUniformLocation("gColorMap");
    m_displacementMapLocation = GetUniformLocation("gDisplacementMap");
    m_eyeWorldPosLocation = GetUniformLocation("gEyeWorldPos");
    m_dirLightLocation.Color = GetUniformLocation("gDirectionalLight.Base.Color");
    m_dirLightLocation.AmbientIntensity = GetUniformLocation("gDirectionalLight.Base.AmbientIntensity");
    m_dirLightLocation.Direction = GetUniformLocation("gDirectionalLight.Direction");
    m_dirLightLocation.DiffuseIntensity = GetUniformLocation("gDirectionalLight.Base.DiffuseIntensity");
    m_matSpecularIntensityLocation = GetUniformLocation("gMatSpecularIntensity");
    m_matSpecularPowerLocation = GetUniformLocation("gSpecularPower");
    m_numPointLightsLocation = GetUniformLocation("gNumPointLights");
    m_numSpotLightsLocation = GetUniformLocation("gNumSpotLights");
    m_dispFactorLocation = GetUniformLocation("gDispFactor");

    if (m_dirLightLocation.AmbientIntensity == INVALID_UNIFORM_LOCATION ||
        m_VPLocation == INVALID_UNIFORM_LOCATION ||
        m_WorldMatrixLocation == INVALID_UNIFORM_LOCATION ||
        m_colorTextureLocation == INVALID_UNIFORM_LOCATION ||
        m_displacementMapLocation == INVALID_UNIFORM_LOCATION ||
        m_eyeWorldPosLocation == INVALID_UNIFORM_LOCATION ||
        m_dirLightLocation.Color == INVALID_UNIFORM_LOCATION ||
        m_dirLightLocation.DiffuseIntensity == INVALID_UNIFORM_LOCATION ||
        m_dirLightLocation.Direction == INVALID_UNIFORM_LOCATION ||
        m_matSpecularIntensityLocation == INVALID_UNIFORM_LOCATION ||
        m_matSpecularPowerLocation == INVALID_UNIFORM_LOCATION ||
        m_numPointLightsLocation == INVALID_UNIFORM_LOCATION ||
        m_numSpotLightsLocation == INVALID_UNIFORM_LOCATION ||
        m_dispFactorLocation == INVALID_UNIFORM_LOCATION) {
        return false;
    }

    for (unsigned int i = 0 ; i < ARRAY_SIZE_IN_ELEMENTS(m_pointLightsLocation) ; i++) {
        char Name[128];
        memset(Name, 0, sizeof(Name));
        SNPRINTF(Name, sizeof(Name), "gPointLights[%d].Base.Color", i);
        m_pointLightsLocation[i].Color = GetUniformLocation(Name);

        SNPRINTF(Name, sizeof(Name), "gPointLights[%d].Base.AmbientIntensity", i);
        m_pointLightsLocation[i].AmbientIntensity = GetUniformLocation(Name);

        SNPRINTF(Name, sizeof(Name), "gPointLights[%d].Position", i);
        m_pointLightsLocation[i].Position = GetUniformLocation(Name);

        SNPRINTF(Name, sizeof(Name), "gPointLights[%d].Base.DiffuseIntensity", i);
        m_pointLightsLocation[i].DiffuseIntensity = GetUniformLocation(Name);

        SNPRINTF(Name, sizeof(Name), "gPointLights[%d].Atten.Constant", i);
        m_pointLightsLocation[i].Atten.Constant = GetUniformLocation(Name);

        SNPRINTF(Name, sizeof(Name), "gPointLights[%d].Atten.Linear", i);
        m_pointLightsLocation[i].Atten.Linear = GetUniformLocation(Name);

        SNPRINTF(Name, sizeof(Name), "gPointLights[%d].Atten.Exp", i);
        m_pointLightsLocation[i].Atten.Exp = GetUniformLocation(Name);

        if (m_pointLightsLocation[i].Color == INVALID_UNIFORM_LOCATION ||
            m_pointLightsLocation[i].AmbientIntensity == INVALID_UNIFORM_LOCATION ||
            m_pointLightsLocation[i].Position == INVALID_UNIFORM_LOCATION ||
            m_pointLightsLocation[i].DiffuseIntensity == INVALID_UNIFORM_LOCATION ||
            m_pointLightsLocation[i].Atten.Constant == INVALID_UNIFORM_LOCATION ||
            m_pointLightsLocation[i].Atten.Linear == INVALID_UNIFORM_LOCATION ||
            m_pointLightsLocation[i].Atten.Exp == INVALID_UNIFORM_LOCATION) {
            return false;
        }
    }

    for (unsigned int i = 0 ; i < ARRAY_SIZE_IN_ELEMENTS(m_spotLightsLocation) ; i++) {
        char Name[128];
        memset(Name, 0, sizeof(Name));
        SNPRINTF(Name, sizeof(Name), "gSpotLights[%d].Base.Base.Color", i);
        m_spotLightsLocation[i].Color = GetUniformLocation(Name);

        SNPRINTF(Name, sizeof(Name), "gSpotLights[%d].Base.Base.AmbientIntensity", i);
        m_spotLightsLocation[i].AmbientIntensity = GetUniformLocation(Name);

        SNPRINTF(Name, sizeof(Name), "gSpotLights[%d].Base.Position", i);
        m_spotLightsLocation[i].Position = GetUniformLocation(Name);

        SNPRINTF(Name, sizeof(Name), "gSpotLights[%d].Direction", i);
        m_spotLightsLocation[i].Direction = GetUniformLocation(Name);

        SNPRINTF(Name, sizeof(Name), "gSpotLights[%d].Cutoff", i);
        m_spotLightsLocation[i].Cutoff = GetUniformLocation(Name);

        SNPRINTF(Name, sizeof(Name), "gSpotLights[%d].Base.Base.DiffuseIntensity", i);
        m_spotLightsLocation[i].DiffuseIntensity = GetUniformLocation(Name);

        SNPRINTF(Name, sizeof(Name), "gSpotLights[%d].Base.Atten.Constant", i);
        m_spotLightsLocation[i].Atten.Constant = GetUniformLocation(Name);

        SNPRINTF(Name, sizeof(Name), "gSpotLights[%d].Base.Atten.Linear", i);
        m_spotLightsLocation[i].Atten.Linear = GetUniformLocation(Name);

        SNPRINTF(Name, sizeof(Name), "gSpotLights[%d].Base.Atten.Exp", i);
        m_spotLightsLocation[i].Atten.Exp = GetUniformLocation(Name);

        if (m_spotLightsLocation[i].Color == INVALID_UNIFORM_LOCATION ||
            m_spotLightsLocation[i].AmbientIntensity == INVALID_UNIFORM_LOCATION ||
            m_spotLightsLocation[i].Position == INVALID_UNIFORM_LOCATION ||
            m_spotLightsLocation[i].Direction == INVALID_UNIFORM_LOCATION ||
            m_spotLightsLocation[i].Cutoff == INVALID_UNIFORM_LOCATION ||
            m_spotLightsLocation[i].DiffuseIntensity == INVALID_UNIFORM_LOCATION ||
            m_spotLightsLocation[i].Atten.Constant == INVALID_UNIFORM_LOCATION ||
            m_spotLightsLocation[i].Atten.Linear == INVALID_UNIFORM_LOCATION ||
            m_spotLightsLocation[i].Atten.Exp == INVALID_UNIFORM_LOCATION) {
            return false;
        }
    }

    return true;
}


void LightingTechnique::SetVP(const Matrix4f& VP)
{
    glUniformMatrix4fv(m_VPLocation, 1, GL_TRUE, (const GLfloat*)VP.m);    
}


void LightingTechnique::SetWorldMatrix(const Matrix4f& WorldInverse)
{
    glUniformMatrix4fv(m_WorldMatrixLocation, 1, GL_TRUE, (const GLfloat*)WorldInverse.m);
}


void LightingTechnique::SetColorTextureUnit(unsigned int TextureUnit)
{
    glUniform1i(m_colorTextureLocation, TextureUnit);
}


void LightingTechnique::SetDisplacementMapTextureUnit(unsigned int TextureUnit)
{
    glUniform1i(m_displacementMapLocation, TextureUnit);
}


void LightingTechnique::SetDirectionalLight(const DirectionalLight& Light)
{
    glUniform3f(m_dirLightLocation.Color, Light.Color.x, Light.Color.y, Light.Color.z);
    glUniform1f(m_dirLightLocation.AmbientIntensity, Light.AmbientIntensity);
    Vector3f Direction = Light.Direction;
    Direction.Normalize();
    glUniform3f(m_dirLightLocation.Direction, Direction.x, Direction.y, Direction.z);
    glUniform1f(m_dirLightLocation.DiffuseIntensity, Light.DiffuseIntensity);
}

void LightingTechnique::SetEyeWorldPos(const Vector3f& EyeWorldPos)
{
    glUniform3f(m_eyeWorldPosLocation, EyeWorldPos.x, EyeWorldPos.y, EyeWorldPos.z);
}

void LightingTechnique::SetMatSpecularIntensity(float Intensity)
{
    glUniform1f(m_matSpecularIntensityLocation, Intensity);
}

void LightingTechnique::SetMatSpecularPower(float Power)
{
    glUniform1f(m_matSpecularPowerLocation, Power);
}

void LightingTechnique::SetPointLights(unsigned int NumLights, const PointLight* pLights)
{
    glUniform1i(m_numPointLightsLocation, NumLights);
    
    for (unsigned int i = 0 ; i < NumLights ; i++) {
        glUniform3f(m_pointLightsLocation[i].Color, pLights[i].Color.x, pLights[i].Color.y, pLights[i].Color.z);
        glUniform1f(m_pointLightsLocation[i].AmbientIntensity, pLights[i].AmbientIntensity);
        glUniform1f(m_pointLightsLocation[i].DiffuseIntensity, pLights[i].DiffuseIntensity);
        glUniform3f(m_pointLightsLocation[i].Position, pLights[i].Position.x, pLights[i].Position.y, pLights[i].Position.z);
        glUniform1f(m_pointLightsLocation[i].Atten.Constant, pLights[i].Attenuation.Constant);
        glUniform1f(m_pointLightsLocation[i].Atten.Linear, pLights[i].Attenuation.Linear);
        glUniform1f(m_pointLightsLocation[i].Atten.Exp, pLights[i].Attenuation.Exp);
    }
}

void LightingTechnique::SetSpotLights(unsigned int NumLights, const SpotLight* pLights)
{
    glUniform1i(m_numSpotLightsLocation, NumLights);

    for (unsigned int i = 0 ; i < NumLights ; i++) {
        glUniform3f(m_spotLightsLocation[i].Color, pLights[i].Color.x, pLights[i].Color.y, pLights[i].Color.z);
        glUniform1f(m_spotLightsLocation[i].AmbientIntensity, pLights[i].AmbientIntensity);
        glUniform1f(m_spotLightsLocation[i].DiffuseIntensity, pLights[i].DiffuseIntensity);
        glUniform3f(m_spotLightsLocation[i].Position,  pLights[i].Position.x, pLights[i].Position.y, pLights[i].Position.z);
        Vector3f Direction = pLights[i].Direction;
        Direction.Normalize();
        glUniform3f(m_spotLightsLocation[i].Direction, Direction.x, Direction.y, Direction.z);
        glUniform1f(m_spotLightsLocation[i].Cutoff, cosf(ToRadian(pLights[i].Cutoff)));
        glUniform1f(m_spotLightsLocation[i].Atten.Constant, pLights[i].Attenuation.Constant);
        glUniform1f(m_spotLightsLocation[i].Atten.Linear,   pLights[i].Attenuation.Linear);
        glUniform1f(m_spotLightsLocation[i].Atten.Exp,      pLights[i].Attenuation.Exp);
    }
}


void LightingTechnique::SetDispFactor(float Factor)
{
    glUniform1f(m_dispFactorLocation, Factor);
}
