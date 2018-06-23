#version 400

in vec3 fragInColor;
out vec4 outColor;

void main()
{
  //fragInColor;
  outColor = vec4(fragInColor, 1.0);
  //outColor = vec4(1.0, 0.0, 0.0, 1.0);
}
