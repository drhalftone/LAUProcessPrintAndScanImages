#version 330 core

// THIS FRAGMENT SHADER APPLIES AN 8X1 MATRIX TRANSFORM TO EIGHT CONSECUTIVE PIXELS PER ROW

// SET THE NUMBER OF FILTERS AND THE LENGTH OF EACH IN WEIGHTS
#define NUMFILTERS  8

uniform sampler2D qt_texture;  // THIS TEXTURE HOLDS THE XYZ+TEXTURE COORDINATES

// DEFINE THE CONVOLUTIONAL FILTER WEIGHTS
const float weights[8] = float[8](-0.608258, -0.443879, -0.745747, -0.245460, -0.692312, 0.313457, -0.502901, 0.267735);

layout(location = 0, index = 0) out vec4 qt_fragColor;

void main()
{
    // GET THE CURRENT FRAGMENT COORDINATE IN PIXELS
    ivec2 position = ivec2(gl_FragCoord.xy);

    // INITIALIZE THE OUTPUT PIXEL TO THE FIRST PIXEL IN THE SWATH
    qt_fragColor = vec4(0.0, 0.0, 0.0, 0.0);

    // ITERATE OVER CURRENT 8 INPUT PIXELS
    for (int c = 0; c < NUMFILTERS; c++){
        qt_fragColor += weights[c] * texelFetch(qt_texture, ivec2(gl_FragCoord.x / NUMFILTERS + c, gl_FragCoord.y), 0);
    }
}
