#version 330 core

// THIS FRAGMENT SHADER TAKES 1 INPUT ROW AND EXPANDS IT TO 8 OUTPUT ROWS
// EACH OUTPUT ROW CORRESPONDS TO THE 1 INPUT ROW BEING CONVOLVED WITH A
// DIFFERENT 16-WEIGHT FIR FILTER.

// SET THE NUMBER OF FILTERS AND THE LENGTH OF EACH IN WEIGHTS
#define NUMFILTERS  8
#define LENFILTERS 16

uniform sampler2D qt_texture;  // THIS TEXTURE HOLDS THE XYZ+TEXTURE COORDINATES

// DEFINE THE CONVOLUTIONAL FILTER WEIGHTS
const float weights[128] = float[128](  1.226920127869,   1.554644465446,   0.943712353706,   0.848060786724,   0.355745017529,  -0.694445788860,  -1.431247472763,  -1.441693305969,  -0.633546173573,   1.609517574310,   1.053263545036,   1.229796051979,   1.129843473434,   1.346761822701,  -0.294192045927,  -1.431528568268,
                                        0.879069983959,   0.803473234177,   0.671798646450,   0.365302324295,  -0.133533731103,  -1.003404974937,  -0.960429549217,  -0.218929111958,   0.875867247581,   0.992749392986,   0.797869265079,   0.582308709621,   0.637349128723,  -0.163655266166,  -0.617889821529,  -0.938229739666,
                                       -1.376200556755,  -0.467068403959,   0.885730862617,   1.378811120987,   1.210414767265,   0.849493920803,   1.026239275932,   0.363229870796,  -1.092139244080,  -1.528380990028,  -1.002324938774,   0.180152818561,   0.884023249149,   0.960678994656,   0.872989296913,   0.906781673431,
                                        0.420639514923,  -1.522188186646,  -1.308977007866,   2.077903985977,   1.733032703400,  -0.036913044751,  -0.046018913388,   1.393929839134,   1.383430957794,  -1.595449566841,  -1.480996847153,  -1.383971929550,   0.893846809864,   1.117269515991,   1.401886463165,   1.367937445641,
                                        1.259068727493,   0.904829561710,   0.086111001670,  -1.509420752525,  -1.667881727219,  -1.277634143829,   1.798301458359,   1.463743805885,   1.365100502968,   1.179507851601,   2.232408523560,  -0.923670887947,  -1.664493799210,  -1.639610409737,   1.210348367691,   1.177318692207,
                                       -0.181890755892,  -0.201340019703,  -0.166873186827,  -0.168044686317,   0.235160604119,   0.152464032173,   0.172057613730,   0.024898668751,  -0.172628477216,  -0.126152992249,  -0.167843744159,   0.064023204148,   0.222564905882,   0.268819510937,   0.263414353132,   0.071134217083,
                                        0.042084239423,  -0.075753599405,  -0.100984603167,  -0.200151622295,  -0.115946725011,   0.227561786771,   0.256581842899,   0.295930922031,   0.241750180721,  -0.227471292019,  -0.229846552014,  -0.385815441608,  -0.102913893759,   0.239115178585,   0.295117020607,   0.324705988169,
                                       -0.414017766714,  -0.727040231228,  -0.843306899071,  -0.607191562653,  -0.127774879336,   0.651911377907,   0.519134640694,   0.325194060802,   0.253439307213,   0.498258143663,   0.105790980160,  -0.614983975887,  -0.534896612167,  -0.419707506895,  -0.512102842331,   0.111343890429);

const float bias[8] = float[8](-0.051734443754,  -0.014176220633,  -0.002488293685,   0.029614562169,   0.030888171867,  -0.040303979069,  -0.036583919078,  -0.001227098168);

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
        qt_fragColor += weights[index * LENFILTERS + c] * texelFetch(qt_texture, ivec2(position.x + c, position.y/NUMFILTERS), 0);
    }

    // ADD IN THE FILTER BIAS
    qt_fragColor += bias[index];
}