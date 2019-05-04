#version 330 core

out vec4 color;
in vec4 coords;

void main()
{
    //color = vec4(116.f, 0.f, 126.f, 1.f);;
    color = coords;
}