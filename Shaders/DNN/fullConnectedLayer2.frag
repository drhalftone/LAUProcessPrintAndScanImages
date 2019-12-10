#version 330 core

// THIS FRAGMENT SHADER APPLIES AN 8X1 MATRIX TRANSFORM TO EIGHT CONSECUTIVE PIXELS PER ROW

uniform sampler2D qt_texture;  // THIS TEXTURE HOLDS THE XYZ+TEXTURE COORDINATES

uniform       int qt_padLo;    // THIS SETS THE NUMBER OF NONZERO OUTPUTS
uniform       int qt_padHi;    // THIS SETS THE NUMBER OF NONZERO + ZERO OUTPUTS

// DEFINE THE FULLY CONNECTED NEURAL NETWORK WEIGHTS
const float weights[8] = float[8](-0.352630496025,  -0.449661403894,  -0.189453303814,  -0.422280848026,   0.894745171070,   0.831288278103,  -0.884626626968,   0.295367211103);

layout(location = 0, index = 0) out vec4 qt_fragColor;

void main()
{
    // GET THE CURRENT FRAGMENT COORDINATE IN PIXELS
    ivec2 position = ivec2(gl_FragCoord.xy);

    // INITIALIZE THE OUTPUT PIXEL TO THE FIRST PIXEL IN THE SWATH
    qt_fragColor = vec4(0.0, 0.0, 0.0, 0.0);

    // DETERMINE WHAT MATRIX ROW TO USE BASED ON COLUMN COORDINATE
    int index = position.x % qt_padHi;

    // ITERATE OVER CURRENT 8 INPUT PIXELS
    for (int c = 0; c < 8; c++){
        qt_fragColor += weights[c] * texelFetch(qt_texture, ivec2(8 * position.x + c, position.y), 0);
    }

    // ADD IN THE FILTER BIAS
    qt_fragColor += 0.064052656293;
}
