#version 330

layout(location = 0) out vec4 FragColor;

in vec2 Tex3;
in float Height;

uniform sampler2D gTextureHeight0;
uniform sampler2D gTextureHeight1;
uniform sampler2D gTextureHeight2;
uniform sampler2D gTextureHeight3;

uniform sampler2D gHeightMap;

uniform float gHeight0 = 80.0;
uniform float gHeight1 = 250.0;
uniform float gHeight2 = 350.0;
uniform float gHeight3 = 450.0;

uniform vec3 gReversedLightDir;

uniform float gColorTexcoordScaling = 16.0;

vec4 CalcTexColor()
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
}


vec3 CalcNormal()
{   
    float left  = textureOffset(gHeightMap, Tex3, ivec2(-1, 0)).r;
    float right = textureOffset(gHeightMap, Tex3, ivec2( 1, 0)).r;
    float up    = textureOffset(gHeightMap, Tex3, ivec2( 0, 1)).r;
    float down  = textureOffset(gHeightMap, Tex3, ivec2( 0, -1)).r;

    vec3 normal = normalize(vec3(left - right, 2.0, up - down));
    
    return normal;
}

void main()
{
    vec4 TexColor = CalcTexColor();

    vec3 Normal = CalcNormal();

    float Diffuse = dot(Normal, gReversedLightDir);

    Diffuse = max(0.2f, Diffuse);

    FragColor = TexColor * Diffuse;
 }
