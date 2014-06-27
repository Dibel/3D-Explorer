uniform sampler2D qt_Texture0;
uniform sampler2D qt_Texture1;
uniform vec4 target;

float max(float a, float b) { return a > b ? a : b; }

bool isTarget(vec2 pos)
{
    vec4 color = texture2D(qt_Texture1, pos);
    if (color.r - target.r > 0.1) return false;
    if (color.g - target.g > 0.1) return false;
    if (color.b - target.b > 0.1) return false;
    if (color.r - target.r < -0.1) return false;
    if (color.g - target.g < -0.1) return false;
    if (color.b - target.b < -0.1) return false;
    return true;
}

void main()
{
    vec2 tc = gl_TexCoord[0].st;
    
    if (isTarget(tc)) {
        gl_FragColor = vec4(0.0);

    } else {
        float a, b, x, y;

        a = texture2D(qt_Texture0, tc).a; x = a * 0.2270270270;
        b = texture2D(qt_Texture0, vec2(tc.s, tc.t - 0.003)).a; if (a < b) a = b; x += a * 0.3891891892;
        b = texture2D(qt_Texture0, vec2(tc.s, tc.t - 0.006)).a; if (a < b) a = b; x += a * 0.2432432432;
        b = texture2D(qt_Texture0, vec2(tc.s, tc.t - 0.009)).a; if (a < b) a = b; x += a * 0.1081081082;
        b = texture2D(qt_Texture0, vec2(tc.s, tc.t - 0.012)).a; if (a < b) a = b; x += a * 0.0324324324;

        a = texture2D(qt_Texture0, tc).a; y = a * 0.2270270270;
        b = texture2D(qt_Texture0, vec2(tc.s, tc.t + 0.003)).a; if (a < b) a = b; y += a * 0.3891891892;
        b = texture2D(qt_Texture0, vec2(tc.s, tc.t + 0.006)).a; if (a < b) a = b; y += a * 0.2432432432;
        b = texture2D(qt_Texture0, vec2(tc.s, tc.t + 0.009)).a; if (a < b) a = b; y += a * 0.1081081082;
        b = texture2D(qt_Texture0, vec2(tc.s, tc.t + 0.012)).a; if (a < b) a = b; y += a * 0.0324324324;

        gl_FragColor = vec4(1.0, 1.0, 1.0, x > y ? x : y);
    }
}
