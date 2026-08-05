#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

uniform sampler2D texture0;
uniform vec2 u_resolution; // 原始纹理的分辨率 (Width, Height)

const float PI = 3.14159265359;
const float A = 3.0; // Lanczos3 (若改为 2.0 即为 Lanczos2)

// Lanczos 权重计算函数
float sinc(float x) {
    if (abs(x) < 0.00001) return 1.0;
    x *= PI;
    return sin(x) / x;
}

float lanczos(float x) {
    x = abs(x);
    if (x < A) {
        return sinc(x) * sinc(x / A);
    }
    return 0.0;
}

void main() {
    // 换算到像素坐标空间
    vec2 texelSize = 1.0 / u_resolution;
    vec2 pixelCoord = fragTexCoord * u_resolution - 0.5;
    
    vec2 ic = floor(pixelCoord);
    vec2 fc = pixelCoord - ic;

    vec4 colorSum = vec4(0.0);
    float weightSum = 0.0;

    // 6x6 采样窗口 (从 -2 到 +3)
    for (int y = -2; y <= 3; y++) {
        for (int x = -2; x <= 3; x++) {
            // 计算当前采样点与目标的相对距离
            vec2 offset = vec2(float(x), float(y));
            vec2 sampleCoord = (ic + offset + 0.5) * texelSize;

            // 相对权重的距离向量
            vec2 delta = offset - fc;
            
            // 计算 2D 独立 separable 权重 (Wx * Wy)
            float w = lanczos(delta.x) * lanczos(delta.y);

            vec4 sampleColor = texture(texture0, sampleCoord);
            
            colorSum += sampleColor * w;
            weightSum += w;
        }
    }

    // 归一化权重输出
    finalColor = (colorSum / weightSum) * fragColor;
}