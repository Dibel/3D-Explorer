attribute highp vec4 qt_Vertex;
attribute highp vec4 qt_MultiTexCoord0;

void main(void) {
    gl_Position = qt_Vertex;
    gl_TexCoord[0] = qt_MultiTexCoord0;
}
