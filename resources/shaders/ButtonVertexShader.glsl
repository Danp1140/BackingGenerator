#version 410 core

layout(location=0) in vec2 vertexposition;

out vec4 colorthru;

uniform int width;
uniform int height;
uniform vec4 colorin;

void main() {
    vec2 adjustedposition=vertexposition/vec2(width/2, height/2);
    adjustedposition-=vec2(1, 1);
    gl_Position=vec4(adjustedposition, 0.0f, 1.0f);
    colorthru=colorin;
}
