#version 330 core

uniform sampler2D qt_texture;  // THIS TEXTURE HOLDS THE XYZ+TEXTURE COORDINATES
uniform       int qt_radius;   // HOLDS THE WIDTH OF THE CURRENT FRAGMENT TARGET

layout(location = 0, index = 0) out vec4 qt_fragColor;

void main()
{
    // GET THE CURRENT FRAGMENT COORDINATE IN PIXELS
    ivec2 position = ivec2(gl_FragCoord.xy);

    // INITIALIZE THE OUTPUT PIXEL TO ZEROS
    qt_fragColor = texelFetch(qt_texture, ivec2(position.x, position.y), 0);

    for (int r = 1; r < qt_radius; r++){
        // GET THE PIXEL COORDINATE OF THE CURRENT FRAGMENT
        qt_fragColor += texelFetch(qt_texture, ivec2(position.x, position.y - r), 0);
        qt_fragColor += texelFetch(qt_texture, ivec2(position.x, position.y + r), 0);
    }
    qt_fragColor = qt_fragColor / (2 * qt_radius + 1);
}
