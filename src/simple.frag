#version 400

in vec3 fragInColor;
out vec4 outColor;

void main()
{
  outColor = vec4(fragInColor, 1.0);
}
