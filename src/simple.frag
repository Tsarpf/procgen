#version 400

in vec3 fragInColor;
in vec3 fragPos;
in vec3 outNormal;

out vec4 outColor;


vec3 lightPos = vec3(0., 10., 0.);
vec3 lightColor = vec3(1., 1., 1.);
vec3 objectColor = vec3(0., 1., 0.);

void main()
{
  //fragInColor;
  vec3 norm = normalize(outNormal);
  vec3 lightDir = normalize(lightPos - fragPos);
  float diff = max(dot(norm, lightDir), 0.0);
  vec3 diffuse = diff * lightColor * fragInColor;
   vec3 result = diffuse * objectColor;

   outColor = vec4(result, 1.0);
  //outColor = vec4(fragInColor, 1.0);

  //outColor = vec4(1.0, 0.0, 0.0, 1.0);
}
