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
uniform float gHeight1 = 210.0;
uniform float gHeight2 = 250.0;
uniform float gHeight3 = 280.0;

uniform vec3 gReversedLightDir;

vec4 CalcTexColor()
{
    vec4 TexColor;

    vec2 ScaledTexCoord = Tex3 * 16.0;
//    float Height = WorldPos.y;

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
    float HEIGHT_SCALE = 1.0;
    float uTexelSize = 1.0 / 256.0;
      
    float left  = texture(gHeightMap, Tex3 + vec2(-uTexelSize, 0.0)).r;// * HEIGHT_SCALE * 2.0 - 1.0;
    float right = texture(gHeightMap, Tex3 + vec2( uTexelSize, 0.0)).r;// * HEIGHT_SCALE * 2.0 - 1.0;
    float up    = texture(gHeightMap, Tex3 + vec2(0.0,  uTexelSize)).r;// * HEIGHT_SCALE * 2.0 - 1.0;
    float down  = texture(gHeightMap, Tex3 + vec2(0.0, -uTexelSize)).r;// * HEIGHT_SCALE * 2.0 - 1.0;
 //   vec3 normal = normalize(vec3(down - up, 2.0, left - right));

 //vec3 normal = normalize(0.25 * vec3(2*(right - left), 2 * (down - up), -4));

 vec3 normal = normalize(vec3(left - right, down - up, 2.0));

    return normal;
}

void main()
{
    vec4 TexColor = CalcTexColor();

    vec3 Normal = CalcNormal();

    float Diffuse = dot(Normal, gReversedLightDir);

    Diffuse = max(0.3f, Diffuse);

    FragColor = TexColor * Diffuse;

 //   FragColor = vec4(Normal, 1.0);
}
