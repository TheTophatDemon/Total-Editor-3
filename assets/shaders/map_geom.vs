#version 330

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;

in mat4 instanceTransform;

// Input uniform values
uniform mat4 mvp;

// Output vertex attributes (to fragment shader)
out vec3 fragPosition;
out vec2 fragTexCoord;


// NOTE: Add here your custom variables

void main()
{
    // Compute MVP for current instance
    mat4 mvpi = mvp*instanceTransform;

    // Send vertex attributes to fragment shader
    fragPosition = vec3(mvpi*vec4(vertexPosition, 1.0));
    fragTexCoord = vertexTexCoord;
    //fragColor = vertexColor;
    //fragNormal = normalize(vertexNormal);

    // Calculate final vertex position
    gl_Position = mvpi*vec4(vertexPosition, 1.0);
}
