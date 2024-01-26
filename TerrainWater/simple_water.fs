#version 330

layout(location = 0) out vec4 FragColor;

in vec2 oTex;
in float oHeight;
in float oMaxHeight;
in vec3 oNormal;

void main()
{
    float Color = oHeight/oMaxHeight;

    vec3 Normal = normalize(oNormal);
    vec3 LightDir = vec3(1.0f, -1.0f, 0.0f);

    float Diffuse = max(dot(Normal, LightDir), 0.0);
    Diffuse = max(Diffuse, 0.3);

    float FinalRatio = max(Diffuse, 0.5);

    FragColor = FinalRatio * vec4(15.0/255.0, 94.0/255.0, 156.0/255.0, 1.0);
}
