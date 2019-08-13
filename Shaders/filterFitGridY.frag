#version 330 core

in vec4 qt_pixel;      // PASSES THE CURRENT PIXEL TO THE FRAGMENT SHADER

layout(location = 0, index = 0) out vec4 qt_fragColor;

void main()
{
    // SET THE CURRENT PIXEL TO THE INCOMING PIXEL
    qt_fragColor = qt_pixel;

    return;
}
