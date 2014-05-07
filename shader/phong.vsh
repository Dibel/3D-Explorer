attribute highp vec4 qt_Vertex;
attribute highp vec4 qt_MultiTexCoord0;

uniform highp mat4 qt_ModelViewProjectionMatrix;
attribute highp vec3 qt_Normal;
uniform highp mat4 qt_ModelViewMatrix;
uniform highp mat3 qt_NormalMatrix;

struct qt_SingleLightParameters {
    mediump vec4 position;
    mediump vec3 spotDirection;
    mediump float spotExponent;
    mediump float spotCutoff;
    mediump float spotCosCutoff;
    mediump float constantAttenuation;
    mediump float linearAttenuation;
    mediump float quadraticAttenuation;
};
uniform qt_SingleLightParameters qt_Light;
// Color to fragment program
varying vec3 vVaryingNormal;
varying vec3 vVaryingLightDir;
varying vec2 vTexCoords;

void main(void)
    {
    // Get surface normal in eye coordinates
    vVaryingNormal = qt_NormalMatrix * qt_Normal;

    // Get vertex position in eye coordinates
    vec4 vPosition4 = qt_ModelViewMatrix * qt_Vertex;
    vec3 vPosition3 = vPosition4.xyz / vPosition4.w;

    // Get vector to light source
    vVaryingLightDir = normalize(qt_Light.position - vPosition3);

    // Pass along the texture coordinates
    vTexCoords = qt_MultiTexCoord0.st;

    // Don't forget to transform the geometry!
    gl_Position = qt_ModelViewProjectionMatrix * qt_Vertex;
    }
