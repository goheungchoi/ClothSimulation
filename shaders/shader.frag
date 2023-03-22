#version 330 core

layout (location=0) out vec4 out_fragcolor;

in vec3 position;
in vec3 normal;
in vec4 color;
in vec2 uv;

uniform int useTexture;
uniform sampler2D textureImage;

void main(){
    
    // hard code some material properties
    float ka = 0.3f;
    float kd = 0.6f;
    
    // L is a unit vector from the fragment location towards this light
    vec3 L1 = -1.f*normalize(vec3(position)-vec3(10.f, 10.f, 10.f));
    vec3 L2 = -1.f*vec3(0.f, 1.f, -1.f);

    // normalize the input normal that was interpolated from the mesh vertices
	vec3 N = normalize(normal);
    
    // compute a simple diffuse shading weight that is agnostic to the order in which the triangle vertices were specified
    float N_dot_L1 = dot(N, L1);
	if ((N_dot_L1) < 0.0) { N_dot_L1 *= -1.0; }

    float N_dot_L2 = dot(N, L2);
	if ((N_dot_L2) < 0.0) { N_dot_L2 *= -1.0; }
    
    // use a simplified ambient+diffuse shading model to define the fragment color
    vec3 i = kd * vec3(color) * N_dot_L1;
    i += kd * vec3(color) * N_dot_L2;
    vec3 result = ka * vec3(color) + i;
	out_fragcolor = vec4( result, 1.0 );

    if (useTexture != 0) {
        out_fragcolor *= texture(textureImage, uv);
    }
} 


