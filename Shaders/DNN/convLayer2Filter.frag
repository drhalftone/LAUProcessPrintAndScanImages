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

uniform       int qt_padLo;     // THIS SETS THE NUMBER OF NONZERO OUTPUTS
uniform       int qt_padHi;     // THIS SETS THE NUMBER OF NONZERO + ZERO OUTPUTS

// DEFINE THE CONVOLUTIONAL FILTER WEIGHTS
const float weights[128] = float[128]( -0.051695380360,   0.119573555887,   0.001400924753,   0.365833997726,   0.060049641877,  -0.116841606796,  -0.290060579777,  -0.100266352296,   0.433955997229,   0.200837284327,  -0.117466010153,  -0.273089140654,   0.296237140894,   0.272497862577,   0.016735369340,  -0.053786814213,
                                        0.010669942945,  -0.175422072411,  -0.121406480670,  -0.033578805625,   0.036610044539,  -0.064456686378,  -0.138360813260,  -0.185606807470,   0.337307989597,   0.364360928535,  -0.059744555503,  -0.116289488971,  -0.044685330242,   0.300561159849,   0.056219872087,  -0.143823251128,
                                        0.183570891619,   0.080141261220,   0.101718023419,  -0.016058923677,  -0.261195778847,  -0.181632250547,   0.407706528902,   0.429719060659,  -0.003460699925,  -0.346061617136,  -0.117700651288,   0.163002014160,   0.321458131075,  -0.207452192903,  -0.277574777603,  -0.199258431792,
                                        0.289209932089,   0.428424686193,   0.379477739334,   0.127533793449,   0.055090956390,   0.112370342016,   0.496612757444,   0.164238408208,   0.132427364588,  -0.077427342534,   0.175862595439,   0.466028064489,   0.318206042051,  -0.038499306887,   0.236146584153,   0.341738641262,
                                        0.289960861206,   0.230238556862,   0.109218679368,   0.008867371827,   0.260555565357,   0.401331037283,   0.386055171490,   0.031320907176,   0.181045576930,   0.308192968369,   0.555901348591,   0.060175795108,  -0.071358010173,   0.328955769539,   0.535978198051,   0.310077816248,
                                        0.024620097131,   0.023270599544,  -0.145445853472,  -0.110926307738,  -0.044337760657,  -0.129865184426,   0.131772398949,  -0.143602579832,  -0.136465236545,  -0.150255665183,  -0.137169972062,  -0.055684320629,  -0.104442059994,   0.156682252884,  -0.053700264543,  -0.166471168399,
                                        0.150057315826,   0.179575771093,  -0.051587831229,  -0.184538602829,  -0.107596196234,  -0.045709788799,   0.019044354558,  -0.136037826538,   0.010813514702,   0.075040824711,  -0.132694706321,  -0.185022786260,  -0.099571123719,  -0.088394448161,  -0.061392929405,  -0.034197371453,
                                       -0.132888689637,  -0.079772658646,   0.086918212473,  -0.233327001333,   0.186204567552,   0.174399018288,   0.043981999159,  -0.024737425148,  -0.160961896181,  -0.009819832630,   0.273819327354,   0.055081654340,  -0.056762233377,  -0.181882679462,  -0.021347172558,   0.093799315393);

layout(location = 0, index = 0) out vec4 qt_fragColor;

void main()
{
    // GET THE CURRENT FRAGMENT COORDINATE IN PIXELS
    ivec2 position = ivec2(gl_FragCoord.xy);

    // INITIALIZE THE OUTPUT PIXEL TO THE FIRST PIXEL IN THE SWATH
    qt_fragColor = vec4(0.0, 0.0, 0.0, 0.0);

    // ITERATE THROUGH ALL EIGHT CHANNELS
    for (int r = 0; r < NUMFILTERS; r++){
        // FIGURE OUT WHAT INPUT ROWS WE WANT BASED ON THE OUTPUT ROW COORDINATE
        int row = 8 * position.y + r;

        // APPLY THE APPROPRIATE FILTER TO THE CURRENT INPUT ROW
        // ADDING IT TO THE CURRENT OUTPUT PIXELS CUMMULATIVE SUM
        for (int c = 0; c < LENFILTERS; c++){
            qt_fragColor += weights[r * LENFILTERS + c] * texelFetch(qt_texture, ivec2(position.x + c, row), 0);
        }
    }

    // ADD IN THE FILTER BIAS
    qt_fragColor += 0.049307160079;

    // APPLY ZERO PADDING
    qt_fragColor *= float((position.x % qt_padHi) < qt_padLo);
}
