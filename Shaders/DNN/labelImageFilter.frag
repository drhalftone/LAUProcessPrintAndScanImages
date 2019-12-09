#version 330 core

uniform sampler2D qt_textureA;   // THIS TEXTURE HOLDS THE INPUT IMAGE
uniform sampler2D qt_textureB;   // THIS TEXTURE HOLDS THE HEADLIFE SCORES

uniform     float qt_threshold;  // HOLDS THE SCORE THRESHOLD ABOVE WHICH A SWATCH IS DECLARED DEAD

layout(location = 0, index = 0) out vec4 qt_fragColor;

void main()
{
    // GET THE CURRENT FRAGMENT COORDINATE IN PIXELS
    ivec2 position = ivec2(gl_FragCoord.xy);

    qt_fragColor = texelFetch(qt_textureA, ivec2(position.x, position.y), 0);

    // READ THE SCORE PIXEL FROM THE SECOND TEXTURE
    float score = texelFetch(qt_textureB, ivec2(position.x, position.y), 0).r;

    // SET PIXELS TO RED IF THEY EXCEED THE THRESHOLD
    if (score > qt_threshold){
        qt_fragColor.r = 1.0;
    }
}
