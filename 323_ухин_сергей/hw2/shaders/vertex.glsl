#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 Normals;
layout (location = 2) in vec2 texCoord;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;
out vec4 FragPosLightSpace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;


void main()
{
	
    gl_Position = projection * view * model * vec4(position.x, position.y, position.z, 1.0f);
    TexCoord = vec2(texCoord.x, texCoord.y);
    FragPos = vec3(model * vec4(position, 1.0f));
    Normal = mat3(model) * Normals;
    FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
    //Normal = vec3(model * vec4(Normals, 0.0f));
}