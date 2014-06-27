uniform sampler2D qt_Texture0;
uniform vec4 target;

bool isTarget(float s, float t)
{
    vec4 color = texture2D(qt_Texture0, vec2(s, t));
    if (color.r - target.r > 0.1) return false;
    if (color.g - target.g > 0.1) return false;
    if (color.b - target.b > 0.1) return false;
    if (color.r - target.r < -0.1) return false;
    if (color.g - target.g < -0.1) return false;
    if (color.b - target.b < -0.1) return false;
    return true;
}

void main() {
    float value = 0.0;
    vec2 tc = gl_TexCoord[0].st;
    float blur = 0.01;
    float hstep = 1.0;
    float vstep = 0.0;

    if (isTarget(tc.s - 4.0 * blur * hstep, tc.t)) value += 0.0162162162;
    if (isTarget(tc.s - 3.0 * blur * hstep, tc.t)) value += 0.0540540541;
    if (isTarget(tc.s - 2.0 * blur * hstep, tc.t)) value += 0.1216216216;
    if (isTarget(tc.s - 1.0 * blur * hstep, tc.t)) value += 0.1945945946;

    if (isTarget(tc.s, tc.t)) value += 0.2270270270;

    if (isTarget(tc.s + 1.0 * blur * hstep, tc.t)) value += 0.1945945946;
    if (isTarget(tc.s + 2.0 * blur * hstep, tc.t)) value += 0.1216216216;
    if (isTarget(tc.s + 3.0 * blur * hstep, tc.t)) value += 0.0540540541;
    if (isTarget(tc.s + 4.0 * blur * hstep, tc.t)) value += 0.0162162162;


    //if (isTarget(texture2D(qt_Texture0, vec2(tc.s - 4.0 * blur * hstep, tc.t)))) value += 0.0162162162;
    //if (isTarget(texture2D(qt_Texture0, vec2(tc.s - 3.0 * blur * hstep, tc.t)))) value += 0.0540540541;
    //if (isTarget(texture2D(qt_Texture0, vec2(tc.s - 2.0 * blur * hstep, tc.t)))) value += 0.1216216216;
    //if (isTarget(texture2D(qt_Texture0, vec2(tc.s - 1.0 * blur * hstep, tc.t)))) value += 0.1945945946;

    //if (isTarget(texture2D(qt_Texture0, tc.st))) value += 0.2270270270;

    //if (isTarget(texture2D(qt_Texture0, vec2(tc.s + 1.0 * blur * hstep, tc.t)))) value += 0.1945945946;
    //if (isTarget(texture2D(qt_Texture0, vec2(tc.s + 2.0 * blur * hstep, tc.t)))) value += 0.1216216216;
    //if (isTarget(texture2D(qt_Texture0, vec2(tc.s + 3.0 * blur * hstep, tc.t)))) value += 0.0540540541;
    //if (isTarget(texture2D(qt_Texture0, vec2(tc.s + 4.0 * blur * hstep, tc.t)))) value += 0.0162162162;

    gl_FragColor = vec4(0.0, 1.0, 1.0, value);




    //vec4 textureColor;

    //bool l = gl_TexCoord[0].s + 0.005 <= 1.0
    //        && !isTarget(texture2D(qt_Texture0, vec2(gl_TexCoord[0].s - 0.001, gl_TexCoord[0].t)))
    //        && isTarget(texture2D(qt_Texture0, vec2(gl_TexCoord[0].s + 0.005, gl_TexCoord[0].t)));
    //bool r = gl_TexCoord[0].s - 0.005 >= 0.0
    //        && !isTarget(texture2D(qt_Texture0, vec2(gl_TexCoord[0].s + 0.001, gl_TexCoord[0].t)))
    //        && isTarget(texture2D(qt_Texture0, vec2(gl_TexCoord[0].s - 0.005, gl_TexCoord[0].t)));
    //bool b = gl_TexCoord[0].t + 0.005 <= 1.0
    //        && !isTarget(texture2D(qt_Texture0, vec2(gl_TexCoord[0].s, gl_TexCoord[0].t - 0.001)))
    //        && isTarget(texture2D(qt_Texture0, vec2(gl_TexCoord[0].s, gl_TexCoord[0].t + 0.005)));
    //bool t = gl_TexCoord[0].t - 0.005 >= 0.0
    //        && !isTarget(texture2D(qt_Texture0, vec2(gl_TexCoord[0].s, gl_TexCoord[0].t + 0.001)))
    //        && isTarget(texture2D(qt_Texture0, vec2(gl_TexCoord[0].s, gl_TexCoord[0].t - 0.005)));
    //bool lb = gl_TexCoord[0].s + 0.005 <= 1.0 && gl_TexCoord[0].t + 0.005 <= 1.0
    //        && !isTarget(texture2D(qt_Texture0, vec2(gl_TexCoord[0].s - 0.001, gl_TexCoord[0].t - 0.001)))
    //        && isTarget(texture2D(qt_Texture0, vec2(gl_TexCoord[0].s + 0.005, gl_TexCoord[0].t + 0.005)));
    //bool lt = gl_TexCoord[0].s + 0.005 <= 1.0 && gl_TexCoord[0].t - 0.005 >= 0.0
    //        && !isTarget(texture2D(qt_Texture0, vec2(gl_TexCoord[0].s - 0.001, gl_TexCoord[0].t + 0.001)))
    //        && isTarget(texture2D(qt_Texture0, vec2(gl_TexCoord[0].s + 0.005, gl_TexCoord[0].t - 0.005)));
    //bool rb = gl_TexCoord[0].s - 0.005 >= 0.0 && gl_TexCoord[0].t + 0.005 <= 1.0
    //        && !isTarget(texture2D(qt_Texture0, vec2(gl_TexCoord[0].s + 0.001, gl_TexCoord[0].t - 0.001)))
    //        && isTarget(texture2D(qt_Texture0, vec2(gl_TexCoord[0].s - 0.005, gl_TexCoord[0].t + 0.005)));
    //bool rt = gl_TexCoord[0].s - 0.005 >= 0.0 && gl_TexCoord[0].t - 0.005 >= 0.0
    //        && !isTarget(texture2D(qt_Texture0, vec2(gl_TexCoord[0].s + 0.001, gl_TexCoord[0].t + 0.001)))
    //        && isTarget(texture2D(qt_Texture0, vec2(gl_TexCoord[0].s - 0.005, gl_TexCoord[0].t - 0.005)));

    //if (l || r || b || t || lb || lt || rb || rt)
    //    textureColor = vec4(0.0, 1.0, 1.0, 1.0);
    //else
    //    textureColor = vec4(0.0, 0.0, 0.0, 0.0);

    //gl_FragColor = textureColor;
}
