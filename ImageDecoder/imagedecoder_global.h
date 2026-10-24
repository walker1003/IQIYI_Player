#pragma once
#ifdef IMAGEDECODER_EXPORTS
#define IMAGEDECODER_API __declspec(dllexport)
#else
#define IMAGEDECODER_API __declspec(dllimport)
#pragma comment(lib, "ImageDecoder.lib")
#endif