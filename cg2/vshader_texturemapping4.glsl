#version 330

layout (location = 0) in  vec4 vPosition;
layout (location = 1) in  vec3 vNormal;
layout (location = 2) in  vec2 vTexCoord;

out vec4 color;
out vec2 texCoord;

uniform mat4 ModelView;
uniform mat4 Projection;

void main() 
{    
    color       = vec4(1.0, 1.0, 1.0, 1.0);
    texCoord    = vTexCoord;
    gl_Position = Projection * ModelView * vPosition;
} 
