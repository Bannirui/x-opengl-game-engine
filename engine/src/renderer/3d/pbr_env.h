//
// Created by rui ding on 2026/6/1.
// Generates IBL environment maps: irradiance, prefiltered env, BRDF LUT
//

#pragma once

#include "x/core/base.h"
#include "x/renderer/texture_cube.h"

#include <glad/glad.h>

class PBREnvironment {
public:
    static Ref<TextureCube> BakeIrradiance(const Ref<TextureCube>& envMap, uint32_t size = 32);
    static Ref<TextureCube> BakePrefilter(const Ref<TextureCube>& envMap, uint32_t size = 128,
                                             uint32_t maxMip = 5);
    static uint32_t BakeBRDFLUT(uint32_t size = 512);

    // 引擎初始化时
    static void InitCaptureResources();
    static void ShutdownCaptureResources();

private:
    static GLuint s_captureFBO;
    static GLuint s_captureRBO;
    static GLuint s_cubeVAO;
    static GLuint s_quadVAO;
};
