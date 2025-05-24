#version 430

in vec3 Position;
in vec3 Normal;
in vec2 TexCoord;

uniform vec4 LightPosition = vec4(0.0f,0.0f,0.0f,1.0f);
uniform vec3 LightIntensity = vec3(1.0f);

uniform vec3 Kd = vec3(0.8f);
uniform vec3 Ka = vec3(0.2f);
uniform vec3 Ks = vec3(0.2f);
uniform float Shininess = 80.0;

layout( binding = 0 ) uniform sampler2D Tex;

layout( location = 0 ) out vec4 FragColor;

vec3 ads( )
{
    vec3 s = normalize( vec3(LightPosition) - Position );
    vec3 v = normalize(vec3(-Position));
    vec3 r = reflect( -s, Normal );

    return
        LightIntensity * ( Ka +
          Kd * max( dot(s, Normal), 0.0 ) +
          Ks * pow( max( dot(r,v), 0.0 ), Shininess ) );
}

void main() {
  vec4 texColor = texture( Tex, TexCoord );
  FragColor = vec4(ads(), 1.0) * texColor;
}
