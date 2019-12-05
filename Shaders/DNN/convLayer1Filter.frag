#version 330 core

// SET THE NUMBER OF FILTERS AND THE LENGTH OF EACH IN WEIGHTS
#define NUMFILTERS  8
#define LENFILTERS 16

uniform sampler2D qt_texture;  // THIS TEXTURE HOLDS THE XYZ+TEXTURE COORDINATES

// DEFINE THE CONVOLUTIONAL FILTER WEIGHTS
const float weights[128] = float[128](-0.375068, -0.256107,  0.393572,  0.513270,  0.454258, -0.585541, -0.549831, -0.408481,  0.191709,  0.504616, -0.264989, -0.006820,  0.538424,  0.635296, -0.397494, -0.702396,
                                       0.308601,  0.608767,  0.375288, -0.272506, -0.498144, -0.716425,  0.293871,  0.260949, -0.578290, -0.459705, -0.325545,  0.540080,  0.354552,  0.417106, -0.393780, -0.464869,
                                       0.577882,  0.002739, -0.530852, -0.342201,  0.532954,  0.486739,  0.500657, -0.479770, -0.349993,  0.228945,  0.251115, -0.336357, -0.495714,  0.426886,  0.502151,  0.175852,
                                       0.288792,  0.384059, -0.054541, -0.556870, -0.643875,  0.018050,  0.278288,  0.147222,  0.077451,  0.385906, -0.226989, -0.440919, -0.019611,  0.024328, -0.222506, -0.172342,
                                       0.397997,  0.119416,  0.361537,  0.602038, -0.516076, -0.856943, -0.339616,  0.280680,  0.124412,  0.281860,  0.275669,  0.194137,  0.310119, -0.163759, -0.316513, -0.200673,
                                       0.931899,  0.647997,  0.493884,  0.047639, -0.614980, -0.695703, -0.535151, -0.098110,  0.608727,  0.571558,  0.586081, -0.105899, -0.391570, -0.816725,  0.120865,  0.385379,
                                      -0.149069,  0.014270, -0.392323, -0.176774,  0.043529,  0.413651,  0.063492,  0.124748,  0.420208,  0.543884,  0.142822, -0.401951, -0.797585, -0.109971,  0.252467,  0.294298,
                                       0.031344,  0.471531, -0.518362, -0.396768,  0.334031,  0.053382, -0.107342,  0.249371, -0.007678,  0.056254,  0.215093, -0.317550, -0.312643,  0.099687,  0.148848, -0.088463);

layout(location = 0, index = 0) out vec4 qt_fragColor;

void main()
{
    // GET THE CURRENT FRAGMENT COORDINATE IN PIXELS
    ivec2 position = ivec2(gl_FragCoord.xy);

    // FIGURE OUT WHAT FILTER WE WANT BASED ON THE OUTPUT ROW COORDINATE
    int index = position.y % NUMFILTERS;

    // INITIALIZE THE OUTPUT PIXEL TO THE FIRST PIXEL IN THE SWATH    
    qt_fragColor = vec4(0.0, 0.0, 0.0, 0.0);
    for (int c = 0; c < LENFILTERS; c++){
        qt_fragColor += weights[index * LENFILTERS + c] * texelFetch(qt_texture, ivec2(gl_FragCoord.x - c, gl_FragCoord.y/NUMFILTERS), 0);
    }
}
