#version 330

in vec3 position;
in vec3 inColor;
out vec3 fragInColor;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

void main()
{
    fragInColor = inColor;
    gl_Position = Projection * View * Model * vec4(position, 1.0f);
}
