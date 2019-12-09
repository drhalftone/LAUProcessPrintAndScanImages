#version 330 core

uniform sampler2D qt_texture;   // THIS TEXTURE HOLDS THE XYZ+TEXTURE COORDINATES
uniform       int qt_width;     // HOLDS THE WIDTH OF THE CURRENT FRAGMENT TARGET

layout(location = 0, index = 0) out vec4 qt_fragColor;

void main()
{
    // GET THE CURRENT FRAGMENT COORDINATE IN PIXELS
    ivec2 position = ivec2(gl_FragCoord.xy);

    // READ THE INPUT PIXEL FROM TEXTURE
    qt_fragColor = texelFetch(qt_texture, ivec2(position.x / qt_width, position.y), 0);
}
