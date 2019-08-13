#version 330 core

uniform sampler2D qt_texture;        // THIS TEXTURE HOLDS THE XYZ+TEXTURE COORDINATES
uniform      vec4 qt_grid;           // HOLDS THE TRANSFORM TO FIND THE GRID BOUNDARIES
in           vec2 qt_coordinate;     // HOLDS THE TEXTURE COORDINATE FROM THE VERTEX SHADER

layout(location = 0, index = 0) out vec4 qt_fragColor;

void main()
{
    // GET THE PIXEL COORDINATE OF THE CURRENT FRAGMENT
    qt_fragColor = texture(qt_texture, qt_coordinate, 0);

    // GET THE FRAGMENT PIXEL COORDINATE
    vec2 posA = (gl_FragCoord.xy - qt_grid.yw) / qt_grid.xz;
    vec2 posB = abs(posA - round(posA));
    vec2 posC = vec2(greaterThan(posB, vec2(0.01, 0.01))) + vec2(lessThan(posA, vec2(0.0, 0.0)));
    vec2 posD = vec2(greaterThan(posC, vec2(0.5, 0.5)));

    qt_fragColor.a = qt_fragColor.a * float((posD.x + posD.y) > 0.5);
}
