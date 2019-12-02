#version 330 core

uniform sampler2D qt_texture;        // THIS TEXTURE HOLDS THE XYZ+TEXTURE COORDINATES
uniform       int rowStep = 1;       // HOW MANY STEPS BETWEEN IMAGE ROWS IN THE INCOMING TEXTURE
uniform       int rowOfst = 0;       // STARTING ROW FOR THE IMAGE TO BE PROCESSED IN THE INCOMING TEXTURE

const float LoD[10] = float[10]( 0.003335725285002,  -0.012580751999016,  -0.006241490213012,   0.077571493840065,  -0.032244869585030,
                                -0.242294887066190,   0.138428145901103,   0.724308528438574,   0.603829269797473,   0.160102397974125 );
const float HiD[10] = float[10](-0.160102397974125,   0.603829269797473,  -0.724308528438574,   0.138428145901103,   0.242294887066190,
                                -0.032244869585030,  -0.077571493840065,  -0.006241490213012,   0.012580751999016,   0.003335725285002 );

layout(location = 0, index = 0) out vec4 qt_fragColor;

void main()
{
    // GET THE CURRENT FRAGMENT COORDINATE IN PIXELS
    ivec2 position = ivec2(gl_FragCoord.xy);

    // INITIALIZE THE OUTPUT PIXEL TO ZEROS
    qt_fragColor = vec4(0.0, 0.0, 0.0, 0.0);

    // GET THE INPUT IMAGE ROW COORDINATE
    int row = rowStep * (position.y / 2) + rowOfst;

    // FIGURE OUT IF WE ARE THE LOW OF HIGH DECOMPOSITION FILTER
    if (position.y % 2 == 0){
        for (int c = 0; c < 10; c++){
            // GET THE CURRENT INPUT PIXEL COLUMN COORDINATE WITH WRAP AROUND
            int col = (position.x - c); //((position.x - c) + textureSize(qt_texture,0).x) % textureSize(qt_texture,0).x;

            // GET THE PIXEL COORDINATE OF THE CURRENT FRAGMENT
            qt_fragColor += LoD[c] * texelFetch(qt_texture, ivec2(col, row), 0);
        }
    } else {
        for (int c = 0; c < 10; c++){
            // GET THE CURRENT INPUT PIXEL COLUMN COORDINATE WITH WRAP AROUND
            int col = (position.x - c); //((position.x - c) + textureSize(qt_texture,0).x) % textureSize(qt_texture,0).x;

            // GET THE PIXEL COORDINATE OF THE CURRENT FRAGMENT
            qt_fragColor += HiD[c] * texelFetch(qt_texture, ivec2(col, row), 0);
        }
    }
}
