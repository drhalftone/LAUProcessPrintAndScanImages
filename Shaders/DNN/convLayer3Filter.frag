#version 330 core

// THIS FRAGMENT SHADER TAKES 1 INPUT ROW AND APPLIES A 16-WEIGHT
// FIR FILTER TO IT, OUTPUT TO 1 OUTPUT ROW

// SET THE NUMBER OF FILTERS AND THE LENGTH OF EACH IN WEIGHTS
#define LENFILTERS 16

uniform sampler2D qt_texture;   // THIS TEXTURE HOLDS THE XYZ+TEXTURE COORDINATES

uniform       int qt_padLo;     // THIS SETS THE NUMBER OF NONZERO OUTPUTS
uniform       int qt_padHi;     // THIS SETS THE NUMBER OF NONZERO + ZERO OUTPUTS

// DEFINE THE CONVOLUTIONAL FILTER WEIGHTS
const float weights[LENFILTERS] = float[LENFILTERS](0.400792211294,   0.025730950758,   0.107204042375,   0.556696236134,  -0.225401923060,   0.445497810841,   0.505782485008,   0.326852291822,  -0.201292261481,  -0.074891448021,  -0.005649564788,   0.328295350075,  -0.152026161551,   0.381219267845,  -0.136470183730,   0.324716687202);

layout(location = 0, index = 0) out vec4 qt_fragColor;

void main()
{
    // GET THE CURRENT FRAGMENT COORDINATE IN PIXELS
    ivec2 position = ivec2(gl_FragCoord.xy);

    // INITIALIZE THE OUTPUT PIXEL TO THE FIRST PIXEL IN THE SWATH    
    qt_fragColor = vec4(0.0, 0.0, 0.0, 0.0);
    for (int c = 0; c < LENFILTERS; c++){
        qt_fragColor += weights[c] * texelFetch(qt_texture, ivec2(position.x + c, position.y), 0);
    }

    // ADD IN THE FILTER BIAS
    qt_fragColor += 0.061739400029;

    // APPLY ZERO PADDING
    qt_fragColor *= float((position.x % qt_padHi) < qt_padLo);
}
