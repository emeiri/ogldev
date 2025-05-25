#version 430

in vec3 Position;
in vec3 Normal;
in vec2 TexCoord;

uniform vec3 LightPosition = vec3(0.0f,0.0f, -1.0f);
uniform vec3 LightIntensity = vec3(2.0f);
uniform vec3 Diffuse = vec3(0.8f);
uniform vec3 Ambient = vec3(0.2f);
uniform vec3 Specular = vec3(0.2f);
uniform float Shininess = 80.0;

layout( binding = 0 ) uniform sampler2D Tex;

layout( location = 0 ) out vec4 FragColor;


void main() 
{
    vec4 TexColor = texture( Tex, TexCoord );
    vec3 PixelToLight = normalize(LightPosition - Position);
    vec3 ViewDir = normalize(-Position);
    vec3 r = reflect(-PixelToLight, Normal);
    
    float DiffuseFactor = max(dot(PixelToLight, Normal), 0.0);
    float SpecularFactor = pow(max(dot(r,ViewDir), 0.0), Shininess);

    vec3 LightColor = LightIntensity * (Ambient + 
                                        Diffuse * DiffuseFactor +                                        
                                        Specular * SpecularFactor);

    FragColor = vec4(LightColor, 1.0) * TexColor;
}
