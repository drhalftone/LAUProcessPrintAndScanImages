#version 330 core

uniform sampler2D qt_texture;  // THIS TEXTURE HOLDS THE XYZ+TEXTURE COORDINATES
uniform       int qt_width;    // HOLDS THE WIDTH OF THE CURRENT FRAGMENT TARGET

const float LoR[10] = float[10]( 0.160102397974125,   0.603829269797473,   0.724308528438574,   0.138428145901103,  -0.242294887066190,
                                -0.032244869585030,   0.077571493840065,  -0.006241490213012,  -0.012580751999016,   0.003335725285002 );
const float HiR[10] = float[10]( 0.003335725285002,   0.012580751999016,  -0.006241490213012,  -0.077571493840065,  -0.032244869585030,
                                 0.242294887066190,   0.138428145901103,  -0.724308528438574,   0.603829269797473,  -0.160102397974125 );

layout(location = 0, index = 0) out vec4 qt_fragColor;

void main()
{
    // GET THE CURRENT FRAGMENT COORDINATE IN PIXELS
    ivec2 position = ivec2(gl_FragCoord.xy);

    // INITIALIZE THE OUTPUT PIXEL TO ZEROS
    qt_fragColor = vec4(0.0, 0.0, 0.0, 0.0);

    // PROCESS THE LOW FREQUENCY WAVELET COEFFICIENTS
    for (int c = 0; c < 10; c++){
        // GET THE CURRENT INPUT PIXEL COLUMN COORDINATE WITH WRAP AROUND
        int col = (position.x + qt_width + 4 - c) % qt_width;

        // GET THE PIXEL COORDINATE OF THE CURRENT FRAGMENT
        qt_fragColor += LoR[c] * texelFetch(qt_texture, ivec2(col / 2, position.y), 0) * float(col % 2 == 0);

        // GET THE PIXEL COORDINATE OF THE CURRENT FRAGMENT
        qt_fragColor += HiR[c] * texelFetch(qt_texture, ivec2((col + qt_width) / 2, position.y), 0) * float(col % 2 == 0);
    }
}
