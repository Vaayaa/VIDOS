/*
 * Land Waves Boii
 * Made addon by Teafella of teafella.com
 */

#ifdef GL_ES
precision mediump float;
#endif
#define PI 3.1415926535897932384626433832795

// IMPORTANT: set to 0 if you are using this shader in GLSLSandbox.com | 1 if on VidBoi
#define VIDBOI 1

#if VIDBOI == 1
uniform vec4 color;
uniform vec2 scale;
uniform vec2 centre;
uniform vec2 inputVal;
uniform float cv0;
uniform float cv1;
uniform float cv2;
varying vec2 tcoord;
uniform int sceneIndex;
uniform sampler2D tex;
uniform sampler2D texFB;
uniform sampler2D texIN;

#else
uniform vec2 mouse;
float cv0 = mouse.x;
float cv1 = mouse.y;
float cv2 = mouse.x - mouse.y;
uniform vec2 resolution;
#endif

uniform float time;

vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main(void)
{
    vec2 uv;
    //vid boi compatibility ( SandboxSpecific Delete on VidBoi)
    #if VIDBOI  == 0
    uv = gl_FragCoord.xy - resolution.xy;
    #else
    uv = vec2(tcoord.x, tcoord.y);
    #endif
    // end vidboi compatiblity
    
    vec2 p = (2.0 * uv) / min(resolution.x, resolution.y) * 1.0;
    
    uv = vec2(uv.x, uv.y);
    
    for(int i = 1; i < 10; i++) {
        p += sin(p.yx * vec2(1.6, 1.1) * float(i + 5) + time/8.5 * float(i) * vec2(3.5 * time/2., 0.5) / 10000. + 5. ) * 0.1;
    }
    float c = (abs(sin(p.y + time * 0.0) + sin(p.x + time * 0.0))) - 0.5;
    
    //Convert to HSV
    vec3 hsvColor  = rgb2hsv(vec3(c));
    
    hsvColor = vec3(cv0 * sin(time/2.) , cos(cv1/.5)   , hsvColor[2]  );
    
    //Convert back to RGB
    vec3 rgbColor = hsv2rgb(hsvColor);
    
    gl_FragColor = vec4(cos(rgbColor[0]) / cv2, rgbColor[1] * uv.x * cv2, rgbColor[2], 1.0);
}