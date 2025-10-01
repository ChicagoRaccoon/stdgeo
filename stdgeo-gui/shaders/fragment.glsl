// Fragment shader: outputs interpolated color for each pixel
#version 330 core

in vec3 fragColor;          // Interpolated color from vertex shader
out vec4 FragColor;         // Final pixel color

uniform vec4 overrideColor; // Optional color override for highlighting

void main() {
    // If overrideColor has alpha > 0, use it; otherwise use vertex color
    if (overrideColor.a > 0.0) {
        FragColor = overrideColor;
    } else {
        FragColor = vec4(fragColor, 1.0);  // RGB from vertex + full alpha
    }
}