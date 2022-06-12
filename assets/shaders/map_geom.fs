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
