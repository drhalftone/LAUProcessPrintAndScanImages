#version 330 core

uniform sampler2D qt_texture;  // THIS TEXTURE HOLDS THE XYZ+TEXTURE COORDINATES

layout(location = 0, index = 0) out vec4 qt_fragColor;

void main()
{
    // GET THE CURRENT FRAGMENT COORDINATE IN PIXELS
    ivec2 position = ivec2(gl_FragCoord.xy);

    // INITIALIZE THE OUTPUT PIXEL TO THE FIRST PIXEL IN THE SWATH    
    vec4 qt_fragColorA = texelFetch(qt_texture, ivec2(2 * gl_FragCoord.x + 0, gl_FragCoord.y), 0);
    vec4 qt_fragColorB = texelFetch(qt_texture, ivec2(2 * gl_FragCoord.x + 1, gl_FragCoord.y), 0);

    // NOW GET THE MAXIMUM PIXEL VALUE OVER THE TWO CONSECUTIVE PIXELS
    qt_fragColor = max(qt_fragColorA, qt_fragColorB);
}
