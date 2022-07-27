/**
 * Copyright (c) 2022 Alexander Lunsford
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

#ifndef MAP_SHADER_H
#define MAP_SHADER_H

const char *MAP_SHADER_INSTANCED_V_SRC = R"SHADER(

#version 330

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

in mat4 instanceTransform;

// Input uniform values
uniform mat4 mvp;

// Output vertex attributes (to fragment shader)
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormal;

// NOTE: Add here your custom variables

void main()
{
    // Compute MVP for current instance
    mat4 mvpi = mvp * instanceTransform;

    // Send vertex attributes to fragment shader
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    mat3 matNormal = mat3(instanceTransform[0].xyz, instanceTransform[1].xyz, instanceTransform[2].xyz);
    fragNormal = normalize(matNormal*vertexNormal);

    // Calculate final vertex position
    gl_Position = mvpi*vec4(vertexPosition, 1.0);
}

)SHADER";

const char *MAP_SHADER_V_SRC = R"SHADER(

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

// NOTE: Add here your custom variables

void main()
{
    // Send vertex attributes to fragment shader
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    mat3 matNormal = mat3(matModel[0].xyz, matModel[1].xyz, matModel[2].xyz);
    fragNormal = normalize(matNormal*vertexNormal);

    // Calculate final vertex position
    gl_Position = mvp*vec4(vertexPosition, 1.0);
}

)SHADER";

const char *MAP_SHADER_F_SRC = R"SHADER(

#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Output fragment color
out vec4 finalColor;

const vec3 lightDir = normalize(vec3(1.0, -1.0, -1.0));

void main()
{

    // Texel color fetching from texture sampler
    vec4 texelColor = texture(texture0, fragTexCoord);
    if (texelColor.a < 0.9) discard;

    float dp = dot(-lightDir, fragNormal);
    float shading = min(1.0, (0.5 + (max(0.0,dp) * 0.5)));

    finalColor = vec4((texelColor*colDiffuse*fragColor).xyz*shading, 1.0);
}

)SHADER";

#endif
