#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

uniform sampler2D texture0;
uniform vec2 u_resolution;

vec4 CubicFilter(float x) {
    vec4 w;
    w.x = -0.5 * x * x * x + x * x - 0.5 * x;
    w.y = 1.5 * x * x * x - 2.5 * x * x + 1.0;
    w.z = -1.5 * x * x * x + 2.0 * x * x + 0.5 * x;
    w.w = 0.5 * x * x * x - 0.5 * x * x;
    return w;
}

vec4 TextureBicubic(sampler2D sampler, vec2 texCoords, vec2 texSize) {
    vec2 texelSize = 1.0 / texSize;
    vec2 coord = texCoords * texSize - 0.5;
    vec2 texPos1 = floor(coord);
    vec2 f = coord - texPos1;

    vec4 wx = CubicFilter(f.x);
    vec4 wy = CubicFilter(f.y);

    vec4 result = vec4(0.0);
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            vec2 sampleCoord = (texPos1 + vec2(float(x) - 1.0, float(y) - 1.0) + 0.5) * texelSize;
            sampleCoord = clamp(sampleCoord, texelSize * 0.5, 1.0 - texelSize * 0.5);
            result += texture(sampler, sampleCoord) * wx[x] * wy[y];
        }
    }

    return result;
}

void main() {
    finalColor = TextureBicubic(texture0, fragTexCoord, u_resolution) * fragColor;
}