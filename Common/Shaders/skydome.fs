#version 410

layout(location = 0) out vec4 FragColor;

uniform sampler2D gSampler;
uniform vec4 gLowColor = vec4(0.6, 0.38, 0.66, 1.0);
uniform vec4 gHighColor = vec4(0.0, 0.15, 0.66, 1.0);

//uniform vec4 gLowColor = vec4(1.0, 0.05, 0.6, 1.0);
//uniform vec4 gHighColor = vec4(0.0, 0.5, 0.8, 1.0);


in vec2 TexCoords0;
in float Height;
in vec3 StarPos;

float Hash( float n ){
    return fract( (1.0 + sin(n)) * 415.92653);
}

float Noise3d( vec3 x ){
    float xhash = Hash(round(400*x.x) * 37.0);
    float yhash = Hash(round(400*x.y) * 57.0);
    float zhash = Hash(round(400*x.z) * 67.0);
    return fract(xhash + yhash + zhash);
}



void main()
{
      vec4 TexColor = texture2D(gSampler, TexCoords0.xy);

      float H = Height;

      if (H < 0.0) {
          H = 0.0;
      }

      //float f = mix(1.0, 0.2, H);

      vec4 SkyColor = mix(gLowColor, gHighColor, H);

    //  float threshold = 0.999;

     // float star_intensity = Noise3d(normalize(StarPos));
        //And we apply a threshold to keep only the brightest areas
    //    if (star_intensity >= threshold){
            //We compute the star intensity
   //         float sun_norm = 0.5;
   //         star_intensity = pow((star_intensity - threshold)/(1.0 - threshold), 6.0)*(sun_norm+0.1);
   //         SkyColor += vec4(star_intensity);
   //     }

      FragColor = TexColor * 0.8;// * SkyColor;
}
