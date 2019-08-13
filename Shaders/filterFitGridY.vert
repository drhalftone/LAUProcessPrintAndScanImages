#version 330 core

uniform sampler2D qt_texture;    // THIS TEXTURE HOLDS THE XYZ+TEXTURE COORDINATES
uniform     float qt_angle;      // HOLDS THE ROTATION ANGLE
uniform      vec2 qt_offset;     // HOLDS THE CENTER OF THE IMAGE

in           vec2 qt_vertex;     // ATTRIBUTE WHICH HOLDS THE ROW AND COLUMN COORDINATES FOR THE CURRENT PIXEL

out          vec4 qt_pixel;      // PASSES THE CURRENT PIXEL TO THE FRAGMENT SHADER

void main(void)
{
    // GET A COPY OF THE CURRENT INPUT PIXEL
    qt_pixel = texelFetch(qt_texture, ivec2(qt_vertex), 0);

    // CALCULATE POSITION ON PAGE RELATIVE TO CENTER
    vec2 pos = qt_vertex - qt_offset;

    // SET THE GL_POSITION INSIDE THE FRAME BUFFER OBJECT
    gl_Position.x = 0.0;
    gl_Position.y = (length(pos) * sin(atan(pos.y, pos.x) + qt_angle)) / qt_offset.y;
    gl_Position.z = 0.0;
    gl_Position.w = 1.0;
}
