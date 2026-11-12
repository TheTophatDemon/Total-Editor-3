/**
 * Copyright (c) 2022-present Alexander Lunsford
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 **/

#ifndef AREA_SHADER_H
#define AREA_SHADER_H

const char *AREA_SHADER_V_SRC = R"SHADER(

#version 330

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

// Input uniform values
uniform mat4 mvp;
uniform mat4 matModel;

// Output vertex attributes (to fragment shader)
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormal;
out vec3 fragWorldCoord;

void main()
{
	fragTexCoord = vertexTexCoord;

    // Send vertex attributes to fragment shader
    fragColor = vertexColor;
    mat3 matNormal = mat3(matModel[0].xyz, matModel[1].xyz, matModel[2].xyz);
    fragNormal = normalize(matNormal*vertexNormal);
	fragWorldCoord = vertexPosition;

    // Calculate final vertex position
    gl_Position = mvp*vec4(vertexPosition, 1.0);
	// Take priority when z-fighting
	gl_Position.z += 0.0001;
}

)SHADER";

const char *AREA_SHADER_F_SRC = R"SHADER(

#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;
in vec3 fragWorldCoord;

// Input uniform values
uniform vec4 colDiffuse;
uniform float time;

// Output fragment color
out vec4 finalColor;

void main()
{
	if (
		fragTexCoord.x < 0.01 || 
		fragTexCoord.y < 0.01 || 
		fragTexCoord.x > 0.99 || 
		fragTexCoord.y > 0.99 || 
		abs(mod((fragWorldCoord.x - fragWorldCoord.y - fragWorldCoord.z) + (time * 0.25), 0.5)) < 0.02
	)
	{
		finalColor = colDiffuse*fragColor;
	}
	else
	{
		discard;
	}
}

)SHADER";

#endif
