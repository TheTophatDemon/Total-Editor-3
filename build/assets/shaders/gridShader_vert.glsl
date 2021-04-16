#version 330 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec4 aColor;

uniform mat4 uViewProjMat;
uniform mat4 uModelMat;

out vec4 vColor;

void main()
{
	gl_Position = uViewProjMat * uModelMat * vec4(aPosition, 1.0);
	vColor = aColor;
}