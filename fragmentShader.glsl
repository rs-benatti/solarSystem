#version 330 core
in vec3 fCol; // available per fragment, interpolated based on your drawing primitive
out vec4 color;
//out vec3 color; // shader output: the color of this fragment
in vec2 UV;

// Values that stay constant for the whole mesh.
uniform sampler2D myTextureSampler;

in vec3 fPos;
in vec3 fNormal;
in vec3 lightDirection;
in vec3 LightDirection_cameraspace;
in vec3 light;


void main() {
vec3 texColor = texture(myTextureSampler, UV).rgb; // sample the texture color
vec3 result = light * texColor;
color = vec4(result, 1.0);

//color = texture(myTextureSampler, UV).rgb;
}