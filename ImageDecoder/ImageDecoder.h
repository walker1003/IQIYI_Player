#pragma once
#include <QString>
#include <vector>
#include <cstdint>
#include "imagedecoder_global.h"

// 封装解码后传给 OpenGL 的帧数据结构
struct IMAGEDECODER_API ImageFrame {
    std::vector<uint8_t> rgbData;
    int width = 0;
    int height = 0;
    bool isValid = false;
};

class IMAGEDECODER_API ImageDecoder {
public:
    ImageDecoder() = default;
    ~ImageDecoder() = default;

    // 核心接口：输入路径，输出 RGB 像素
    static ImageFrame decode(const QString& filePath);
};