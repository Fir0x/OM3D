#version 450

#include "utils.glsl"

#define WINDOW_WIDTH 1600
#define WINDOW_HEIGHT 900

layout(location = 0) out vec3 out_color;

layout(binding = 0) uniform sampler2D in_color;
layout(binding = 1) uniform sampler2D in_normal;

layout(binding = 0) uniform Data {
    FrameData frame;
};

vec3 remapNormal(vec3 normal) {
    return normalize(normal * 2.0 - vec3(1.0));
}

void main() {
    vec3 color = texelFetch(in_color, ivec2(gl_FragCoord.xy), 0).xyz;
    vec3 normal = remapNormal(texelFetch(in_normal, ivec2(gl_FragCoord.xy), 0).xyz);

    float sun_factor = max(0.0, dot(normal, frame.sun_dir));

    color *= sun_factor * frame.sun_color;

    out_color = color;
}