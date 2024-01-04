#version 400

uniform vec3 gCameraPos;

layout ( location = 0 ) out vec4 FragColor;

in vec2 TexCoord;


float distance_from_sphere(in vec3 p, in vec3 c, float r)
{
    return length(p - c) - r;
}


vec3 ray_marching(vec3 ViewVec)
{
    float total_distance_traveled = 0.0;
    const int NUMBER_OF_STEPS = 32;
    const float MINIMUM_HIT_DISTANCE = 0.001;
    const float MAXIMUM_TRACE_DISTANCE = 1000.0;

    for (int i = 0; i < NUMBER_OF_STEPS; ++i)
    {
        vec3 current_position = gCameraPos + total_distance_traveled * ViewVec;

        float distance_to_closest = distance_from_sphere(current_position, vec3(0.0), 1.0);

        if (distance_to_closest < MINIMUM_HIT_DISTANCE) 
        {
            return vec3(1.0, 0.0, 0.0);
        }

        if (total_distance_traveled > MAXIMUM_TRACE_DISTANCE)
        {
            break;
        }
        total_distance_traveled += distance_to_closest;
    }

    return vec3(0.0);
}

void main()
{
    vec3 ViewVec = vec3(TexCoord, 1.0);

    vec3 Color = ray_marching(ViewVec);

    FragColor = vec4(Color, 1.0);
    //FragColor = gColor * 0.001 + vec4(TexCoord, 0.0, 1.0) + vec4(gCameraPos, 1.0);
}
