uniform sampler2D qt_Texture0;

void main() {
    mediump vec4 textureColor;

    bool l = texture2D(qt_Texture0, vec2(gl_TexCoord[0].s - 0.001, gl_TexCoord[0].t)).r == 0.0
          && texture2D(qt_Texture0, vec2(gl_TexCoord[0].s + 0.005, gl_TexCoord[0].t)).r == 1.0;
    bool r = texture2D(qt_Texture0, vec2(gl_TexCoord[0].s + 0.001, gl_TexCoord[0].t)).r == 0.0
          && texture2D(qt_Texture0, vec2(gl_TexCoord[0].s - 0.005, gl_TexCoord[0].t)).r == 1.0;
    bool b = texture2D(qt_Texture0, vec2(gl_TexCoord[0].s, gl_TexCoord[0].t - 0.001)).r == 0.0
          && texture2D(qt_Texture0, vec2(gl_TexCoord[0].s, gl_TexCoord[0].t + 0.005)).r == 1.0;
    bool t = texture2D(qt_Texture0, vec2(gl_TexCoord[0].s, gl_TexCoord[0].t + 0.001)).r == 0.0
          && texture2D(qt_Texture0, vec2(gl_TexCoord[0].s, gl_TexCoord[0].t - 0.005)).r == 1.0;
    bool lb = texture2D(qt_Texture0, vec2(gl_TexCoord[0].s - 0.001, gl_TexCoord[0].t - 0.001)).r == 0.0
           && texture2D(qt_Texture0, vec2(gl_TexCoord[0].s + 0.005, gl_TexCoord[0].t + 0.005)).r == 1.0;
    bool lt = texture2D(qt_Texture0, vec2(gl_TexCoord[0].s - 0.001, gl_TexCoord[0].t + 0.001)).r == 0.0
           && texture2D(qt_Texture0, vec2(gl_TexCoord[0].s + 0.005, gl_TexCoord[0].t - 0.005)).r == 1.0;
    bool rb = texture2D(qt_Texture0, vec2(gl_TexCoord[0].s + 0.001, gl_TexCoord[0].t - 0.001)).r == 0.0
           && texture2D(qt_Texture0, vec2(gl_TexCoord[0].s - 0.005, gl_TexCoord[0].t + 0.005)).r == 1.0;
    bool rt = texture2D(qt_Texture0, vec2(gl_TexCoord[0].s + 0.001, gl_TexCoord[0].t + 0.001)).r == 0.0
           && texture2D(qt_Texture0, vec2(gl_TexCoord[0].s - 0.005, gl_TexCoord[0].t - 0.005)).r == 1.0;

    if (l || r || b || t || lb || lt || rb || rt)
        textureColor = vec4(0.0, 1.0, 1.0, 1.0);
    else
        textureColor = vec4(0.0, 0.0, 0.0, 0.0);

    gl_FragColor = textureColor;
}
