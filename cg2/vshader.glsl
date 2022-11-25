#version 330

layout (location = 0) in vec4 vPosition;

out vec4 color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 segColor;

void main()
{
	gl_Position = projection * view * model * vPosition;
	color = vec4(segColor, 1.0);
}