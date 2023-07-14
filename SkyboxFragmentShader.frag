#version 330 core

in vec3 TexCoords;

out vec4 FragColor;

uniform samplerCube skyboxTextures;

void main()
{
	FragColor = texture(skyboxTextures, TexCoords);
}