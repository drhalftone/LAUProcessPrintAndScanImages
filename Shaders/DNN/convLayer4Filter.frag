#version 330 core

// THIS FRAGMENT SHADER TAKES 1 INPUT ROW AND APPLIES A 16-WEIGHT
// FIR FILTER TO IT, OUTPUT TO 1 OUTPUT ROW

// SET THE NUMBER OF FILTERS AND THE LENGTH OF EACH IN WEIGHTS
#define LENFILTERS 16

uniform sampler2D qt_texture;  // THIS TEXTURE HOLDS THE XYZ+TEXTURE COORDINATES

// DEFINE THE CONVOLUTIONAL FILTER WEIGHTS
const float weights[LENFILTERS] = float[LENFILTERS](0.269713, 0.258248, 0.246852, 0.227863, 0.224730, 0.205908, 0.208107, 0.207844, 0.207073, 0.212334, 0.224435, 0.224092, 0.233129, 0.235973, 0.247705, 0.263668);

layout(location = 0, index = 0) out vec4 qt_fragColor;

void main()
{
    // GET THE CURRENT FRAGMENT COORDINATE IN PIXELS
    ivec2 position = ivec2(gl_FragCoord.xy);

    // INITIALIZE THE OUTPUT PIXEL TO THE FIRST PIXEL IN THE SWATH
    qt_fragColor = vec4(0.0, 0.0, 0.0, 0.0);
    for (int c = 0; c < LENFILTERS; c++){
        qt_fragColor += weights[c] * texelFetch(qt_texture, ivec2(gl_FragCoord.x - c, gl_FragCoord.y), 0);
    }
}
