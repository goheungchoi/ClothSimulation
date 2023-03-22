#version 330 core

layout(location=0) in vec3 in_position;
layout(location=1) in vec3 in_normal;
layout(location=2) in vec3 in_color;
layout(location=3) in vec3 in_texCoord;

out vec3 position;
out vec3 normal;
out vec4 color;
out vec2 uv;

// transformation matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 normalMatrix;

void main()
{
    position = vec3(model * vec4(in_position, 1.f));
    normal = normalize(in_normal);
    color = vec4(in_color, 1.f);
    uv = in_texCoord.xy;
    // apply the model, view, and projection transformations to the vertex position value that will be sent to the clipper, rasterizer, ...
    gl_Position = (projection * view * vec4(position, 1.f));
}
