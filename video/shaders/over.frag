#version 450
// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.
// A game's objects alone over another game's stage (port/gpu_picture.cpp, DrawKept; fight/guest_stages.h): the objects'
// pixels over the stage's, but the stage's pixels in front of the fighters (half there in its picture: colours.frag)
// over the objects that are not the panel's (half there in theirs, when the panel is told from the rest)
layout(location = 0) out vec4 colour;
layout(set = 2, binding = 0) uniform sampler2D stage;
layout(set = 2, binding = 1) uniform sampler2D objects;

void main() {
    const ivec2 at = ivec2(gl_FragCoord.xy);
    const vec4 s = texelFetch(stage, at, 0), o = texelFetch(objects, at, 0);
    const bool mine = o.a > 0.25, panel = o.a > 0.75, front = s.a < 0.75;
    colour = vec4(mine && (panel || !front) ? o.rgb : s.rgb, 1.0);
}
