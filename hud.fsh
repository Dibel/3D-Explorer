uniform sampler2D qt_Texture0;

void main(void)
{
    mediump vec4 textureColor = texture2D(qt_Texture0, gl_TexCoord[0].st);
    gl_FragColor = textureColor;
}
