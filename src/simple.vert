#version 400\

layout(location = 0) in vec3 position;

in vec3 inColor;
out vec3 fragInColor;

void main()
{
    fragInColor = inColor;
    gl_Position = vec4(position, 1.0);
}
