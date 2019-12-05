#version 330 core

uniform sampler2D qt_textureA;  // THIS TEXTURE HOLDS THE XYZ+TEXTURE COORDINATES
uniform sampler2D qt_textureB;  // THIS TEXTURE HOLDS THE XYZ+TEXTURE COORDINATES
uniform       int qt_width;     // HOLDS THE WIDTH OF THE CURRENT FRAGMENT TARGET

layout(location = 0, index = 0) out vec4 qt_fragColor;

void main()
{
    // GET THE CURRENT FRAGMENT COORDINATE IN PIXELS
    ivec2 position = ivec2(gl_FragCoord.xy);

    // READ THE INPUT PIXEL FROM TEXTURE A
    qt_fragColor = texelFetch(qt_textureA, ivec2(position.x, position.y), 0);

    // SUBTRACT THE MEAN PIXEL FROM TEXTURE B
    qt_fragColor = qt_fragColor - texelFetch(qt_textureB, ivec2(position.x / qt_width, position.y), 0);
}
