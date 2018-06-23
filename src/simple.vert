#version 330

in vec3 position;
in vec3 inColor;
out vec3 fragInColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform mat4 mvp;

void main()
{
    fragInColor = inColor;
    //gl_Position = model * vec4(position.xy, 0, 1.0f);
    //gl_Position = model * vec4(position.xy, 0.0f, 1.0f);

    //gl_Position = model * view * proj;

    //gl_Position = proj * view * model * vec4(position.xy, 0.0f, 1.0f);

    gl_Position = mvp * vec4(position, 1.0f);
    //gl_Position = mvp * vec4(position, 1.0f);
}
