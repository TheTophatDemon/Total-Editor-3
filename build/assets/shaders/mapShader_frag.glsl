#version 330 core

in vec2 vUV;
in vec4 vColor;
in vec3 vNormal;

uniform sampler2D uTexture;

out vec4 FragColor;

void main()
{
    FragColor = texture2D(uTexture, vUV * vec2(1.0, -1.0));
}