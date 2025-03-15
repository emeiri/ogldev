// Copied from "The 3D graphics Rendering cookbook 2nd edition":
// https://github.com/PacktPublishing/3D-Graphics-Rendering-Cookbook-Second-Edition/blob/main/Chapter04/04_CubeMap/src/skybox.frag

#version 460 core

in vec3 dir;

out vec4 FragColor;

uniform samplerCube gCubemapTexture;

void main() {
	FragColor = texture(gCubemapTexture, dir);
};
