// Vertex shader: transforms 3D positions and passes color to fragment shader
#version 330 core

layout (location = 0) in vec3 aPos;    // Vertex position
layout (location = 1) in vec3 aColor;  // Vertex color

out vec3 fragColor;  // Pass color to fragment shader

uniform mat4 MVP;    // Model-View-Projection matrix

void main() {
    gl_Position = MVP * vec4(aPos, 1.0);  // Transform position to clip space
    fragColor = aColor;                    // Pass color through unchanged
}