attribute highp vec4 qt_Vertex;
attribute highp vec3 qt_Normal;
uniform mediump mat4 qt_ModelViewProjectionMatrix;

void main(void)
{

   vec4 position = vec4((qt_Vertex.xyz + qt_Normal * 0.4), 1.0);
   position = qt_ModelViewProjectionMatrix * position;
   gl_Position = vec4(position);
}
