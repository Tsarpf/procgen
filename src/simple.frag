#version 400

uniform vec3 cameraPos; // Add camera position as a uniform

in vec3 fragInColor;
in vec3 fragPos;
in vec3 outNormal;

out vec4 outColor;

// Directional light parameters
vec3 lightDir = normalize(vec3(0.0, -1.0, -1.0));
vec3 lightColor = vec3(1.0, 1.0, 1.0);

// Adjusted light settings
vec3 ambient = vec3(0.100, 0.001, 0.001);

void main()
{
  vec3 norm = normalize(outNormal);
  vec3 lightDirection = normalize(lightDir);
  float diff = max(dot(norm, lightDirection), 0.0);
  vec3 diffuse = diff * lightColor;
  vec3 result = ambient + diffuse;
  outColor = vec4(result * fragInColor, 1.0);
}
// with specular etc
// void main()
// {
//   vec3 norm = normalize(outNormal);
//   vec3 lightDirection = normalize(lightDir);

//   // Calculate the diffuse light component
//   float diff = max(dot(norm, lightDirection), 0.0);
//   vec3 diffuse = diff * lightColor;

//   // Specular component
//   vec3 viewDir = normalize(cameraPos - fragPos); // Use actual camera position
//   vec3 reflectDir = reflect(-lightDirection, norm);
//   float spec = pow(max(dot(norm, reflectDir), 0.0), 32.0); // Use Phong model
//   vec3 specular = spec * lightColor * 0.8;

//   // Rim lighting
//   float rim = pow(1.0 - max(dot(viewDir, norm), 0.0), 2.0);
//   vec3 rimColor = rim * vec3(0.5, 0.5, 0.5);

//   Combine ambient, diffuse, specular, and rim lighting
//   vec3 result = ambient + diffuse + specular + rimColor;

//   // Apply the lighting to the fragment color
//   outColor = vec4(result * fragInColor, 1.0);
// }

// //diffuse debug
// void main()
// {
//     vec3 norm = normalize(outNormal);
//     vec3 lightDirection = normalize(lightDir);
//     float diff = max(dot(norm, lightDirection), 0.0);
//     vec3 diffuse = diff * lightColor;
    
//     // Debug: Output diffuse component only
//     outColor = vec4(diffuse, 1.0);
// }

// normals debug
// void main()
// {
//     vec3 norm = normalize(outNormal);
//     outColor = vec4(norm * 0.5 + 0.5, 1.0); // Normals scaled and biased to [0, 1] range
// }


// Point light
/*
#version 400

in vec3 fragInColor;
in vec3 fragPos;
in vec3 outNormal;

out vec4 outColor;


vec3 lightPos = vec3(0., 512., 0.);
vec3 lightColor = vec3(1., 1., 1.);
vec3 objectColor = vec3(0., 1., 0.);

vec3 ambient = vec3(0.3, 0.3, 0.3);

void main()
{
  //fragInColor;

  vec3 norm = normalize(outNormal);
  vec3 lightDir = normalize(lightPos - fragPos);
  float diff = max(dot(norm, lightDir), 0.0);
  vec3 diffuse = diff * lightColor;
  vec3 result = (ambient + diffuse) * fragInColor;
  
  //vec3 result = (ambient + diffuse) * objectColor;

  outColor = vec4(result, 1.0);
  //outColor = vec4(fragInColor, 1.0);

  //outColor = vec4(1.0, 0.0, 0.0, 1.0);
}

*/