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

void main()
{
    bool b;
    float x = 0.0, y = 0.0;
    vec2 tc = gl_TexCoord[0].st;

    b = isTarget(tc.s, tc.t); if (b) x += 0.2270270270;
    b = b || isTarget(tc.s - 0.003, tc.t); if (b) x += 0.3891891892;
    b = b || isTarget(tc.s - 0.006, tc.t); if (b) x += 0.2432432432;
    b = b || isTarget(tc.s - 0.009, tc.t); if (b) x += 0.1081081082;
    b = b || isTarget(tc.s - 0.012, tc.t); if (b) x += 0.0324324324;

    b = isTarget(tc.s, tc.t); if (b) y += 0.2270270270;
    b = b || isTarget(tc.s + 0.003, tc.t); if (b) y += 0.3891891892;
    b = b || isTarget(tc.s + 0.006, tc.t); if (b) y += 0.2432432432;
    b = b || isTarget(tc.s + 0.009, tc.t); if (b) y += 0.1081081082;
    b = b || isTarget(tc.s + 0.012, tc.t); if (b) y += 0.0324324324;

    gl_FragColor = vec4(1.0, 1.0, 1.0, x > y ? x : y);
}
