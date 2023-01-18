#version 450

#include "utils.glsl"

layout(location = 0) out vec3 out_color;

layout(binding = 0) uniform sampler2D in_color;
layout(binding = 1) uniform sampler2D in_normal;

layout(binding = 0) uniform Data {
    FrameData frame;
};

layout(binding = 1) buffer PointLights {
    PointLight point_lights[];
};


vec3 remapNormal(vec3 normal) {
    return normalize(normal * 2.0 - vec3(1.0));
}

vec3 unproject(vec2 uv, float depth, mat4 inv_viewproj) {
    const vec3 ndc = vec3(uv * 2.0 - vec2(1.0), depth);
    const vec4 p = inv_viewproj * vec4(ndc, 1.0);
    return p.xyz / p.w;
}

void main() {
    vec3 color = texelFetch(in_color, ivec2(gl_FragCoord.xy), 0).xyz;
    vec3 normal = remapNormal(texelFetch(in_normal, ivec2(gl_FragCoord.xy), 0).xyz);

    float sun_factor = max(0.0, dot(normal, frame.sun_dir));

    color *= sun_factor * frame.sun_color;

    if (frame.sun_dir == vec3(0.0))
        color = vec3(1.0, 0.0, 0.0);

    out_color = color;
}