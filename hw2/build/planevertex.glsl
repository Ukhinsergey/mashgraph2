#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec2 texCoord;

out vec3 ourColor;
out vec2 TexCoord;
uniform float time;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
void main()
{
	
    gl_Position = projection * view * model * vec4(position.x, position.y, position.z, 1.0f);
    ourColor = vec3(116.f, 0.f, 126.f);
    TexCoord = vec2(texCoord.x, 1.0f - texCoord.y);
}