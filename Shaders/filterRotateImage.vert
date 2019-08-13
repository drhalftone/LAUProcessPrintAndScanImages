#version 330 core

uniform mat4 qt_transform;
in      vec4 qt_vertex;       // POINTS TO VERTICES PROVIDED BY USER ON CPU
out     vec2 qt_coordinate;   // OUTPUT COORDINATE TO FRAGMENT SHADER

void main(void)
{
    // COPY THE VERTEX COORDINATE TO THE GL POSITION
    gl_Position = qt_transform * qt_vertex;
    qt_coordinate = (vec2(sign(qt_vertex.x), sign(qt_vertex.y)) + 1.0)/2.0;
}
