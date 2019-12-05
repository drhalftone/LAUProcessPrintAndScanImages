#version 330 core

uniform sampler2D qt_texture;  // THIS TEXTURE HOLDS THE XYZ+TEXTURE COORDINATES
uniform       int qt_width;   // HOLDS THE WIDTH OF THE CURRENT FRAGMENT TARGET

layout(location = 0, index = 0) out vec4 qt_fragColor;

void main()
{
    // GET THE CURRENT FRAGMENT COORDINATE IN PIXELS
    ivec2 position = ivec2(gl_FragCoord.xy);

    // INITIALIZE THE OUTPUT PIXEL TO THE FIRST PIXEL IN THE SWATH
    qt_fragColor = texelFetch(qt_texture, ivec2(position.x, position.y), 0);

    // AVERAGE THE PIXELS IN THE CURRENT SWATH
    for (int c = 1; c < qt_width; c++){
        qt_fragColor += texelFetch(qt_texture, ivec2(position.x + c, position.y), 0);
    }
    qt_fragColor = qt_fragColor / qt_width;
}
