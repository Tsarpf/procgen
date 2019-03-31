#version 330

in vec3 position;
in vec3 inColor;
in vec3 normal;
out vec3 fragInColor;
out vec3 outNormal;
out vec3 fragPos;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

uniform mat4 mvp;

vec3 lightPos = vec3(0., 10., 0.);

void main()
{
	outNormal = normal;
	fragInColor = inColor;
    //fragInColor = vec3(1.0, 0.0, 0.0);

    gl_Position = Projection * View * Model * vec4(position, 1.0f);
    // gl_Position = mvp * vec4(position, 1.0f);
	//gl_Position = vec4(position, 1.0f);
    //gl_Position = Model * vec4(position, 1.0f);

	fragPos = vec3(Model * vec4(position, 1.0));
}
