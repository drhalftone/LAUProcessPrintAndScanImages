#version 330 core

// THIS FRAGMENT SHADER TAKES 8 INPUT ROWS AND APPLIES A SEPARATE
// 16-WEIGHT FIR FILTER TO IT.  IT THEN ADDS THE OUTPUTS OF EACH
// FIR FILTER AND ADDS THEM TOGETHER TO CREATE 1 OUTPUT ROW

// EACH OUTPUT ROW CORRESPONDS TO THE 1 INPUT ROW BEING CONVOLVED WITH A
// DIFFERENT 16-WEIGHT FIR FILTER.

// SET THE NUMBER OF FILTERS AND THE LENGTH OF EACH IN WEIGHTS
#define NUMFILTERS  8
#define LENFILTERS 16

uniform sampler2D qt_texture;  // THIS TEXTURE HOLDS THE XYZ+TEXTURE COORDINATES

// DEFINE THE CONVOLUTIONAL FILTER WEIGHTS
const float weights[128] = float[128](  0.048892, 0.052087, 0.040520, 0.049857, 0.052646, 0.047626, 0.052612, 0.051947, 0.056661, 0.047002, 0.054492, 0.051447, 0.049186, 0.052586, 0.050129, 0.048890,
                                        0.050106, 0.050351, 0.053086, 0.050724, 0.052953, 0.052091, 0.051398, 0.053059, 0.045234, 0.058207, 0.049976, 0.053280, 0.050974, 0.053350, 0.052799, 0.051726,
                                        -0.338225, -0.275521, -0.308177, -0.351799, -0.289858, -0.301767, -0.321304, -0.307966, -0.296924, -0.299443, -0.271394, -0.332497, -0.307033, -0.285518, -0.315826, -0.278260,
                                        -0.366174, -0.285592, -0.363445, -0.362488, -0.302509, -0.342476, -0.355636, -0.340245, -0.303342, -0.310998, -0.356097, -0.343085, -0.297973, -0.383635, -0.348180, -0.252004,
                                        -0.321003, -0.380268, -0.318532, -0.302302, -0.362136, -0.301217, -0.316739, -0.318519, -0.322468, -0.350227, -0.341810, -0.280251, -0.320873, -0.344421, -0.317876, -0.329708,
                                        0.060467, 0.050685, 0.049993, 0.049589, 0.046890, 0.051273, 0.051939, 0.050602, 0.049491, 0.049800, 0.051089, 0.049573, 0.048594, 0.048473, 0.051931, 0.048057,
                                        -0.247143, -0.292599, -0.380383, -0.290104, -0.305103, -0.342676, -0.295310, -0.279514, -0.318167, -0.335767, -0.292022, -0.277446, -0.358827, -0.343568, -0.236054, -0.329983,
                                        0.050707, 0.058382, 0.051407, 0.050556, 0.051893, 0.051344, 0.051328, 0.042024, 0.050436, 0.051952, 0.057940, 0.050510, 0.050538, 0.050725, 0.052065, 0.053691);

layout(location = 0, index = 0) out vec4 qt_fragColor;

void main()
{
    // GET THE CURRENT FRAGMENT COORDINATE IN PIXELS
    ivec2 position = ivec2(gl_FragCoord.xy);

    // INITIALIZE THE OUTPUT PIXEL TO THE FIRST PIXEL IN THE SWATH
    qt_fragColor = vec4(0.0, 0.0, 0.0, 0.0);
    for (int r = 0; r < NUMFILTERS; r++){
        // FIGURE OUT WHAT INPUT ROWS WE WANT BASED ON THE OUTPUT ROW COORDINATE
        int row = 8 * position.y + r;

        // APPLY THE APPROPRIATE FILTER TO THE CURRENT INPUT ROW
        // ADDING IT TO THE CURRENT OUTPUT PIXELS CUMMULATIVE SUM
        for (int c = 0; c < LENFILTERS; c++){
            qt_fragColor += weights[r * LENFILTERS + c] * texelFetch(qt_texture, ivec2(gl_FragCoord.x - c, gl_FragCoord.y), 0);
        }
    }
}
