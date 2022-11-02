#version 330

layout (location = 0) in vec2 vPosition;
layout (location = 1) in vec3 vColor;
out vec4 color;

uniform mat4 model_view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * vec4(vPosition.xy, 0.0, 1.0);
	color = vec4(vColor, 1.0);
}