#version 330

layout(location = 0) out vec4 FragColor;

in vec2 TexCoord0;
in vec3 WorldPos0;
in float Height;

uniform sampler2D gTextureHeight0;  // albedo
uniform sampler2D gTextureHeight1;  // roughness
uniform sampler2D gTextureHeight2;  // metallic
uniform sampler2D gTextureHeight3;  // normal map

uniform sampler2D gHeightMap;

uniform float gHeight0 = 80.0;
uniform float gHeight1 = 250.0;
uniform float gHeight2 = 350.0;
uniform float gHeight3 = 450.0;

uniform vec3 gReversedLightDir;

uniform vec3 gCameraWorldPos;

uniform float gColorTexcoordScaling = 16.0;

#define PI 3.1415926535897932384626433832795

/*vec4 CalcTexColor()
{
    vec4 TexColor;

    vec2 ScaledTexCoord = Tex3 * gColorTexcoordScaling;

    if (Height < gHeight0) {
       TexColor = texture(gTextureHeight0, ScaledTexCoord);
    } else if (Height < gHeight1) {
       vec4 Color0 = texture(gTextureHeight0, ScaledTexCoord);
       vec4 Color1 = texture(gTextureHeight1, ScaledTexCoord);
       float Delta = gHeight1 - gHeight0;
       float Factor = (Height - gHeight0) / Delta;
       TexColor = mix(Color0, Color1, Factor);
    } else if (Height < gHeight2) {
       vec4 Color0 = texture(gTextureHeight1, ScaledTexCoord);
       vec4 Color1 = texture(gTextureHeight2, ScaledTexCoord);
       float Delta = gHeight2 - gHeight1;
       float Factor = (Height - gHeight1) / Delta;
       TexColor = mix(Color0, Color1, Factor);
    } else if (Height < gHeight3) {
       vec4 Color0 = texture(gTextureHeight2, ScaledTexCoord);
       vec4 Color1 = texture(gTextureHeight3, ScaledTexCoord);
       float Delta = gHeight3 - gHeight2;
       float Factor = (Height - gHeight2) / Delta;
       TexColor = mix(Color0, Color1, Factor);
    } else {
       TexColor = texture(gTextureHeight3, ScaledTexCoord);
    }

    return TexColor;
}*/


vec3 CalcNormal()
{   
    vec2 ScaledTexCoord = TexCoord0 * gColorTexcoordScaling;

    float left  = textureOffset(gHeightMap, ScaledTexCoord, ivec2(-1, 0)).r;
    float right = textureOffset(gHeightMap, ScaledTexCoord, ivec2( 1, 0)).r;
    float up    = textureOffset(gHeightMap, ScaledTexCoord, ivec2( 0, 1)).r;
    float down  = textureOffset(gHeightMap, ScaledTexCoord, ivec2( 0, -1)).r;

    vec3 normal = normalize(vec3(left - right, 2.0, up - down));
    
    return normal;
}


vec3 schlickFresnel(float vDotH, vec3 Albedo)
{
    vec3 F0 = vec3(0.04);    

    vec2 ScaledTexCoord = TexCoord0 * gColorTexcoordScaling;
    
    float Metallic = texture(gTextureHeight2, ScaledTexCoord.xy).x;
    F0 = mix(F0, Albedo, Metallic);

    vec3 ret = F0 + (1 - F0) * pow(clamp(1.0 - vDotH, 0.0, 1.0), 5);

    return ret;
}


float geomSmith(float dp, float Roughness)
{
    float k = (Roughness + 1.0) * (Roughness + 1.0) / 8.0;
    float denom = dp * (1 - k) + k;
    return dp / denom;
}


float ggxDistribution(float nDotH, float Roughness)
{
    float alpha2 = Roughness * Roughness * Roughness * Roughness;
    float d = nDotH * nDotH * (alpha2 - 1) + 1;
    float ggxdistrib = alpha2 / (PI * d * d);
    return ggxdistrib;
}


vec3 getNormalFromMap()
{
    vec2 ScaledTexCoord = TexCoord0 * gColorTexcoordScaling;

    vec3 tangentNormal = texture(gTextureHeight3, ScaledTexCoord).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(WorldPos0);
    vec3 Q2  = dFdy(WorldPos0);
    vec2 st1 = dFdx(ScaledTexCoord);
    vec2 st2 = dFdy(ScaledTexCoord);

    vec3 Normal0 = CalcNormal();
    vec3 N   = normalize(Normal0);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}



vec3 CalcPBRLighting(vec3 Normal)
{
    vec3 LightIntensity = vec3(1.0);

    vec2 ScaledTexCoord = TexCoord0 * gColorTexcoordScaling;

    vec3 l = gReversedLightDir;

    vec3 n = Normal;
    vec3 v = normalize(gCameraWorldPos - WorldPos0);
    vec3 h = normalize(v + l);

    float nDotH = max(dot(n, h), 0.0);
    float vDotH = max(dot(v, h), 0.0);
    float nDotL = max(dot(n, l), 0.0);
    float nDotV = max(dot(n, v), 0.0);

    vec3 fLambert = pow(texture(gTextureHeight0, ScaledTexCoord.xy).xyz, vec3(2.2));

    vec3 F = schlickFresnel(vDotH, fLambert);

    vec3 kS = F;
    vec3 kD = 1.0 - kS;

    float Roughness = texture(gTextureHeight1, ScaledTexCoord.xy).x;

    vec3 SpecBRDF_nom  = ggxDistribution(nDotH, Roughness) *
                         F *
                         geomSmith(nDotL, Roughness) *
                         geomSmith(nDotV, Roughness);

    float SpecBRDF_denom = 4.0 * nDotV * nDotL + 0.0001;

    vec3 SpecBRDF = SpecBRDF_nom / SpecBRDF_denom;

    vec3 DiffuseBRDF = kD * fLambert / PI;

    vec3 FinalColor = (DiffuseBRDF + SpecBRDF) * LightIntensity * nDotL;

    return FinalColor;
}


vec3 CalcPBRDirectionalLight(vec3 Normal)
{
    return CalcPBRLighting(Normal);
}


/*vec3 CalcPBRPointLight(PointLight l, vec3 Normal)
{
    return CalcPBRLighting(l.Base, l.LocalPos, false, Normal);
}*/


void main()
{
    vec3 Normal = getNormalFromMap();

    vec3 TotalLight = CalcPBRDirectionalLight(Normal);

   // for (int i = 0 ;i < gNumPointLights ;i++) {
  //      TotalLight += CalcPBRPointLight(gPointLights[i], Normal);
 //   }

    // HDR tone mapping
    TotalLight = TotalLight / (TotalLight + vec3(1.0));

    // Gamma correction
    FragColor = vec4(pow(TotalLight, vec3(1.0/2.2)), 1.0);
}
