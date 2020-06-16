#version 330 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec2 aUV;
layout (location = 2) in vec4 aColor;
layout (location = 3) in vec3 aNormal;

uniform mat4 uViewProjMat;
uniform mat4 uModelMat;

out vec2 vUV;
out vec4 vColor;
out vec3 vNormal;

void main()
{
	gl_Position = uViewProjMat * uModelMat * vec4(aPosition, 1.0);
	vUV = aUV;
	vColor = aColor;
	vNormal = aNormal;
}