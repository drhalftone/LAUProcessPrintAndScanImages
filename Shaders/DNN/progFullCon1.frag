#version 330 core

// THIS FRAGMENT SHADER APPLIES AN 8X8 MATRIX TRANSFORM TO EIGHT CONSECUTIVE PIXELS PER ROW

// SET THE NUMBER OF FILTERS AND THE LENGTH OF EACH IN WEIGHTS
#define NUMFILTERS  8

uniform sampler2D qt_texture;  // THIS TEXTURE HOLDS THE XYZ+TEXTURE COORDINATES

// DEFINE THE CONVOLUTIONAL FILTER WEIGHTS
const float weights[64] = float[64](-0.608258, -0.443879, -0.745747, -0.245460, -0.692312, 0.313457, -0.502901, 0.267735,
                                    -0.352354, -0.179294, -0.422086, 0.424246, 0.353138, 0.630027, 0.636912, 0.538277,
                                    0.806856, 0.529634, -0.081363, -0.464563, 0.395133, 0.083855, 0.005723, 0.607467,
                                    -0.383007, -0.003189, -0.315467, -0.444344, 0.349292, 0.617006, -0.019055, -0.172754,
                                    0.695886, 0.410867, 0.621571, 0.666759, 0.646240, 0.726887, 0.012990, 0.364633,
                                    0.605181, 0.321764, 0.387908, 0.108737, 0.124844, 0.464114, -0.417410, 0.668289,
                                    0.476494, 0.301514, 0.447009, -0.147379, -0.249566, 0.303215, 0.660447, 0.537804,
                                    -0.284561, -0.365616, 0.223813, 0.206254, 0.371323, 0.442642, -0.356883, 0.137223);

layout(location = 0, index = 0) out vec4 qt_fragColor;

void main()
{
    // GET THE CURRENT FRAGMENT COORDINATE IN PIXELS
    ivec2 position = ivec2(gl_FragCoord.xy);

    // DETERMINE WHAT MATRIX ROW TO USE BASED ON COLUMN COORDINATE
    int index = position.x % NUMFILTERS;

    // INITIALIZE THE OUTPUT PIXEL TO THE FIRST PIXEL IN THE SWATH
    qt_fragColor = vec4(0.0, 0.0, 0.0, 0.0);

    // ITERATE OVER CURRENT 8 INPUT PIXELS
    for (int c = 0; c < NUMFILTERS; c++){
        qt_fragColor += weights[NUMFILTERS * index + c] * texelFetch(qt_texture, ivec2(gl_FragCoord.x / NUMFILTERS + c, gl_FragCoord.y), 0);
    }
}
