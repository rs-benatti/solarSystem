#version 330 core
in vec2 UV;
in vec3 fCol; // available per fragment, interpolated based on your drawing primitive

uniform sampler2D myTextureSampler;

out vec4 color; // shader output: the color of this fragment
void main() {
//color = vec4(1.0);
color = texture( myTextureSampler, UV ).rgb;
}