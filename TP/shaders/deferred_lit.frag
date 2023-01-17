#version 450

layout(location = 0) out vec4 out_color;

layout(location=0) in vec2 out_uv;

layout(binding = 0) uniform sampler2D in_color;
layout(binding = 0) uniform sampler2D in_normal;

vec3 remapNormal(vec3 normal) {
    return 0.0 + (normal + 1.0) * (1.0 - 0.0) / (1.0 + 1.0);
}

void main() {
    out_color = texture(in_color, out_uv);
    out_color.w = 1.0;
}