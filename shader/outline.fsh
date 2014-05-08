uniform sampler2D qt_Texture0;

void main() {
    mediump vec4 textureColor;

    bool l = gl_TexCoord[0].s + 0.005 <= 1.0
            && texture2D(qt_Texture0, vec2(gl_TexCoord[0].s - 0.001, gl_TexCoord[0].t)).r == 0.0
            && texture2D(qt_Texture0, vec2(gl_TexCoord[0].s + 0.005, gl_TexCoord[0].t)).r == 1.0;
    bool r = gl_TexCoord[0].s - 0.005 >= 0.0
            && texture2D(qt_Texture0, vec2(gl_TexCoord[0].s + 0.001, gl_TexCoord[0].t)).r == 0.0
            && texture2D(qt_Texture0, vec2(gl_TexCoord[0].s - 0.005, gl_TexCoord[0].t)).r == 1.0;
    bool b = gl_TexCoord[0].t + 0.005 <= 1.0
            && texture2D(qt_Texture0, vec2(gl_TexCoord[0].s, gl_TexCoord[0].t - 0.001)).r == 0.0
            && texture2D(qt_Texture0, vec2(gl_TexCoord[0].s, gl_TexCoord[0].t + 0.005)).r == 1.0;
    bool t = gl_TexCoord[0].t - 0.005 >= 0.0
            && texture2D(qt_Texture0, vec2(gl_TexCoord[0].s, gl_TexCoord[0].t + 0.001)).r == 0.0
            && texture2D(qt_Texture0, vec2(gl_TexCoord[0].s, gl_TexCoord[0].t - 0.005)).r == 1.0;
    bool lb = gl_TexCoord[0].s + 0.005 <= 1.0 && gl_TexCoord[0].t + 0.005 <= 1.0
            && texture2D(qt_Texture0, vec2(gl_TexCoord[0].s - 0.001, gl_TexCoord[0].t - 0.001)).r == 0.0
            && texture2D(qt_Texture0, vec2(gl_TexCoord[0].s + 0.005, gl_TexCoord[0].t + 0.005)).r == 1.0;
    bool lt = gl_TexCoord[0].s + 0.005 <= 1.0 && gl_TexCoord[0].t - 0.005 >= 0.0
            && texture2D(qt_Texture0, vec2(gl_TexCoord[0].s - 0.001, gl_TexCoord[0].t + 0.001)).r == 0.0
            && texture2D(qt_Texture0, vec2(gl_TexCoord[0].s + 0.005, gl_TexCoord[0].t - 0.005)).r == 1.0;
    bool rb = gl_TexCoord[0].s - 0.005 >= 0.0 && gl_TexCoord[0].t + 0.005 <= 1.0
            && texture2D(qt_Texture0, vec2(gl_TexCoord[0].s + 0.001, gl_TexCoord[0].t - 0.001)).r == 0.0
            && texture2D(qt_Texture0, vec2(gl_TexCoord[0].s - 0.005, gl_TexCoord[0].t + 0.005)).r == 1.0;
    bool rt = gl_TexCoord[0].s - 0.005 >= 0.0 && gl_TexCoord[0].t - 0.005 >= 0.0
            && texture2D(qt_Texture0, vec2(gl_TexCoord[0].s + 0.001, gl_TexCoord[0].t + 0.001)).r == 0.0
            && texture2D(qt_Texture0, vec2(gl_TexCoord[0].s - 0.005, gl_TexCoord[0].t - 0.005)).r == 1.0;

    if (l || r || b || t || lb || lt || rb || rt)
        textureColor = vec4(0.0, 1.0, 1.0, 1.0);
    else
        textureColor = vec4(0.0, 0.0, 0.0, 0.0);

    gl_FragColor = textureColor;
}
