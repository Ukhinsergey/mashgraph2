#version 330 core
in vec3 ourColor;
in vec2 TexCoord;

out vec4 color;

uniform sampler2D ourTexture3;

void main()
{
    color = texture(ourTexture3, TexCoord);
    //color = vec4(ourColor, 1.f);
}