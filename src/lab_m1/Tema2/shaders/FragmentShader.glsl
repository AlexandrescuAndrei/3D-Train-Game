#version 330

in vec3 frag_color;

uniform float time_factor;

layout(location = 0) out vec4 out_color;

void main()
{
    vec3 baseColor = frag_color;
    
    vec3 alertColor = vec3(1.0, 0.0, 0.0);

    vec3 color = mix(baseColor, alertColor, time_factor);

    out_color = vec4(color, 1.0);
}
