#version 330 core
layout(location=0) in vec3 vPos; // input vertex position
layout(location=1) in vec3 vNormal; // input vertex color
layout(location=2) in vec2 vertexUV; // UV mapping
//uniform mat4 viewMat, projMat, translationMatrix;
uniform mat4 transformationMatrix;
uniform mat4 viewMatrix;
uniform mat4 M;
uniform int sunFlag;
uniform vec3 camPos;

out vec3 fPos;
out vec3 fNormal; // output to the next stage, will be rasterized thus available per fragment
out vec3 LightDirection_cameraspace;
out vec2 UV;
out vec3 lightDirection;
out vec3 EyeDirection_cameraspace; 
out vec3 light;
void main() {




fPos = vPos;

lightDirection = vec3(0.0f, 0.0f, 0.0f) - fPos;
fNormal = vNormal; // pass the vertex coordinates to the next stage
UV = vertexUV;
gl_Position = transformationMatrix * vec4(vPos, 1.0); // mandatory to rasterize properly
// Vector that goes from the vertex to the light, in camera space. M is ommited because it's identity.

// In camera space, the camera is at the origin (0,0,0).
vec3 vertexPosition_cameraspace = (viewMatrix * M * vec4(vPos,1)).xyz;
EyeDirection_cameraspace = vec3(0,0,0) - vertexPosition_cameraspace;


vec3 LightPosition_worldspace = vec3(0.0f, 0.0f, 0.0f);
vec3 LightPosition_cameraspace = (viewMatrix * vec4(LightPosition_worldspace,1)).xyz;
LightDirection_cameraspace = LightPosition_cameraspace + EyeDirection_cameraspace;

vec3 n = normalize(vec3(transpose(inverse(M)) * vec4(fNormal, 1.0f)));
vec3 l = normalize(LightDirection_cameraspace);

// ambiente
vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);
float ambientStrength = 0.5f;
vec3 ambient = ambientStrength * lightColor;

// difusa
float diff = max(dot(n, l), 0.0);
vec3 diffuse = diff * lightColor;

// especular
vec3 v = normalize(camPos - fPos);
vec3 r = (2*dot(n, l)*n) - l;
int shininess = 16;
float specular_constant = 2.0f;
float spec = pow(max(dot(v, r), 0.0), shininess);
vec3 specular = specular_constant * spec * lightColor;


if (sunFlag == 0){
	light = (diffuse + ambient + specular);
} else {
	light = vec3(1.0f);
} 

}
