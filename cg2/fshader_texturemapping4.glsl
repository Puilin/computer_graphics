
#version 330

in  vec4 color;
in  vec2 texCoord;

out vec4 fColor;

uniform sampler2D mainTex;
uniform sampler2D cloudTex;
uniform sampler2D terrainTex;

void main() 
{ 
    vec4 earthColor = texture2D(mainTex, texCoord);
    vec4 cloudRed = texture2D(cloudTex, texCoord);
    vec4 terrainColor = texture2D(terrainTex, texCoord);
    vec4 cloudColor = vec4(cloudRed.x, cloudRed.x, cloudRed.x, cloudRed.a);
    fColor = earthColor * terrainColor + cloudColor;
} 
