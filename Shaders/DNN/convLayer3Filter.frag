#version 330 core

// THIS FRAGMENT SHADER TAKES 1 INPUT ROW AND APPLIES A 16-WEIGHT
// FIR FILTER TO IT, OUTPUT TO 1 OUTPUT ROW

// SET THE NUMBER OF FILTERS AND THE LENGTH OF EACH IN WEIGHTS
#define LENFILTERS 16

uniform sampler2D qt_texture;  // THIS TEXTURE HOLDS THE XYZ+TEXTURE COORDINATES

// DEFINE THE CONVOLUTIONAL FILTER WEIGHTS
const float weights[LENFILTERS] = float[LENFILTERS](-0.241045, -0.237482, -0.227275, -0.228335, -0.228409, -0.219225, -0.221898, -0.224422, -0.220480, -0.225995, -0.226502, -0.224388, -0.239700, -0.237258, -0.222406, -0.231869);

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
