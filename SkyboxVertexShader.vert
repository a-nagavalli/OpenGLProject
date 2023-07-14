#version 330 core

layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 view;
uniform mat4 projection;

void main()
{
	TexCoords = aPos;
	vec4 vec = projection * view * vec4(aPos, 1.0);		// no model matrix; multiplying by the model matrix would just translate the box, but we want it to be at the origin
	gl_Position = vec.xyww;		// optimization; sets the z component of the position equal to its w component
								// when perspective division is applied, the resulting depth value will be 1.0
								// so the skybox will be behind everything else (we are rendering the skybox last)
}
