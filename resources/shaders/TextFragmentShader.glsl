#version 410 core

in vec4 colorthru;
in vec2 uvthru;

out vec4 color;

uniform sampler2D texturesampler;

void main() {
    color=vec4(1, 1, 1, texture(texturesampler, uvthru).r)*colorthru;
}
