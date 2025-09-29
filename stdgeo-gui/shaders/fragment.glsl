// Fragment shader: outputs interpolated color for each pixel
#version 330 core

in vec3 fragColor;     // Interpolated color from vertex shader
out vec4 FragColor;    // Final pixel color

void main() {
    FragColor = vec4(fragColor, 1.0);  // RGB from vertex + full alpha
}