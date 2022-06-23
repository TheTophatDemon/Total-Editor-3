#ifndef MAP_SHADER_H
#define MAP_SHADER_H

const char *MAP_SHADER_V_SRC = R"SHADER(

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
out vec3 fragPosition;
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormal;

// NOTE: Add here your custom variables

void main()
{
    // Compute MVP for current instance
    mat4 mvpi = mvp * instanceTransform;

    // Send vertex attributes to fragment shader
    fragPosition = vec3(mvpi*vec4(vertexPosition, 1.0));
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    mat3 matNormal = mat3(instanceTransform[0].xyz, instanceTransform[1].xyz, instanceTransform[2].xyz);
    fragNormal = normalize(matNormal*vertexNormal);

    // Calculate final vertex position
    gl_Position = mvpi*vec4(vertexPosition, 1.0);
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

    float dp = dot(-lightDir, fragNormal);
    float shading = min(1.0, (0.5 + (max(0.0,dp) * 0.5)));

    finalColor = vec4((texelColor*colDiffuse*fragColor).xyz*shading, 1.0);
}

)SHADER";

#endif