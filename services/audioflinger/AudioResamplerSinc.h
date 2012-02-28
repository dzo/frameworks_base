/*
 * Copyright (C) 2007 The Android Open Source Project
 * Copyright (c) 2012, Code Aurora Forum. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_AUDIO_RESAMPLER_SINC_H
#define ANDROID_AUDIO_RESAMPLER_SINC_H

#include <stdint.h>
#include <sys/types.h>
#include <cutils/log.h>

#include "AudioResampler.h"

namespace android {


typedef const int32_t * (*readCoefficientsFn)(bool upDownSample);
typedef int32_t  (*readResampleFirNumCoeffFn)();
typedef int32_t  (*readResampleFirLerpIntBitsFn)();

// ----------------------------------------------------------------------------

class AudioResamplerSinc : public AudioResampler {
public:
    AudioResamplerSinc(int bitDepth, int inChannelCount, int32_t sampleRate, int32_t quality = HIGH_QUALITY);

    ~AudioResamplerSinc();

    virtual void resample(int32_t* out, size_t outFrameCount,
            AudioBufferProvider* provider);
private:
    void init();

    template<int CHANNELS>
    void resample(int32_t* out, size_t outFrameCount,
            AudioBufferProvider* provider);

    template<int CHANNELS>
    inline void filterCoefficient(
            int32_t& l, int32_t& r, uint32_t phase, int16_t const *samples);

    template<int CHANNELS>
    inline void interpolate(
            int32_t& l, int32_t& r,
            int32_t const* coefs, int16_t lerp, int16_t const* samples);

    template<int CHANNELS>
    inline void read(int16_t*& impulse, uint32_t& phaseFraction,
            int16_t const* in, size_t inputIndex);

    readCoefficientsFn mReadResampleCoefficients ;
    readResampleFirNumCoeffFn mReadResampleFirNumCoeff;
    readResampleFirLerpIntBitsFn mReadResampleFirLerpIntBits;

    int16_t *mState;
    int16_t *mImpulse;
    int16_t *mRingFull;

    int32_t const * mFirCoefs;
    static const int32_t mFirCoefsDown[];
    static const int32_t mFirCoefsUp[];

    void * mResampleCoeffLib;
    // ----------------------------------------------------------------------------
    static const int32_t RESAMPLE_FIR_NUM_COEF       = 8;
    static const int32_t RESAMPLE_FIR_LERP_INT_BITS  = 4;

    // we have 16 coefs samples per zero-crossing
    static int coefsBits;
    static int cShift;
    static uint32_t cMask;

    // and we use 15 bits to interpolate between these samples
    // this cannot change because the mul below rely on it.
    static const int pLerpBits = 15;
    static int pShift;
    static uint32_t pMask;

    // number of zero-crossing on each side
    static  unsigned int halfNumCoefs;
};

// ----------------------------------------------------------------------------
}; // namespace android

#endif /*ANDROID_AUDIO_RESAMPLER_SINC_H*/
