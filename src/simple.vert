#version 330

in vec3 position;
in vec3 inColor;
out vec3 fragInColor;

uniform mat4 mvp;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

void main()
{
    fragInColor = inColor;
    //gl_Position = model * vec4(position.xy, 0, 1.0f);
    //gl_Position = model * vec4(position.xy, 0.0f, 1.0f);

    //gl_Position = model * view * proj;


    gl_Position = mvp * vec4(position, 1.0f);
    //gl_Position = vec4(position, 1.0f) * proj * view * model; * 

    //gl_Position = vec4(position, 1.0f) * (proj * view * model) ;
    //gl_Position = Projection * View * Model * vec4(position, 1.0f);
}
