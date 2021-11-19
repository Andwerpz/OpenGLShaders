#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

uniform mat4 model; //global transformation
uniform mat4 view;  //camera transformation
uniform mat4 projection;    //projecting points onto screen
uniform float scale;    //how much bigger does this object get

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoords;

void main()
{
    Normal = mat3(transpose(inverse(model))) * aNormal; //black magic right here
    
    vec3 temp = vec3(Normal.xyz);
    temp = normalize(temp) * scale;

    FragPos = vec3(model * vec4(aPos + temp, 1.0));
    gl_Position = projection * view * model * vec4(aPos + temp, 1.0);
    TexCoords = aTexCoords;
} 