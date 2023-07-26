#ifndef SPRITE_SHADER_H
#define SPRITE_SHADER_H

const char *SPRITE_SHADER_V_SRC = R"SHADER(

#version 330

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

// Input uniform values
uniform mat4 matModel;
uniform mat4 matProj;
uniform mat4 matView;

// Output vertex attributes (to fragment shader)
out vec2 fragTexCoord;
out vec4 fragColor;

void main()
{
    // Send vertex attributes to fragment shader
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;

    vec4 pos = matView * matModel * vec4(0.0, 0.0, 0.0, 1.0);
    pos += vec4(vertexPosition.x * matModel[0][0], vertexPosition.y * matModel[0][0], 0.0, 0.0);
    pos = matProj * pos;

    // Calculate final vertex position
    gl_Position = pos;
}

)SHADER";

const char *SPRITE_SHADER_F_SRC = R"SHADER(

#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Output fragment color
out vec4 finalColor;

void main()
{
    // Texel color fetching from texture sampler
    vec4 texelColor = texture(texture0, fragTexCoord);
    if (texelColor.a < 0.9) discard;

    finalColor = vec4((texelColor*colDiffuse*fragColor).xyz, 1.0);
}

)SHADER";

#endif
