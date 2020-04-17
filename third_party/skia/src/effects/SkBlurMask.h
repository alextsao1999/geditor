/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlurMask_DEFINED
#define SkBlurMask_DEFINED

#include "SkBlurTypes.h"
#include "SkShader.h"
#include "SkMask.h"
#include "SkRRect.h"

class SK_API SkBlurMask {
public:
    static bool BlurRect(SkScalar sigma, SkMask *dst, const SkRect &src, SkBlurStyle,
                         SkIPoint *margin = NULL,
                         SkMask::CreateMode createMode =
                                                SkMask::kComputeBoundsAndRenderImage_CreateMode);
    static bool BlurRRect(SkScalar sigma, SkMask *dst, const SkRRect &src, SkBlurStyle,
                         SkIPoint *margin = NULL,
                         SkMask::CreateMode createMode =
                                                SkMask::kComputeBoundsAndRenderImage_CreateMode);

    // forceQuality will prevent BoxBlur from falling back to the low quality approach when sigma
    // is very small -- this can be used predict the margin bump ahead of time without completely
    // replicating the internal logic.  This permits not only simpler caching of blurred results,
    // but also being able to predict precisely at what pixels the blurred profile of e.g. a
    // rectangle will lie.

    static bool BoxBlur(SkMask* dst, const SkMask& src,
                        SkScalar sigma, SkBlurStyle style, SkBlurQuality quality,
                        SkIPoint* margin = NULL, bool force_quality=false);

    // the "ground truth" blur does a gaussian convolution; it's slow
    // but useful for comparison purposes.
    static bool BlurGroundTruth(SkScalar sigma, SkMask* dst, const SkMask& src, SkBlurStyle,
                                SkIPoint* margin = NULL);

    // If radius > 0, return the corresponding sigma, else return 0
    static SkScalar ConvertRadiusToSigma(SkScalar radius);
    // If sigma > 0.5, return the corresponding radius, else return 0
    static SkScalar ConvertSigmaToRadius(SkScalar sigma);

    /* Helper functions for analytic rectangle blurs */

    /** Look up the intensity of the (one dimnensional) blurred half-plane.
        @param profile The precomputed 1D blur profile; memory allocated by and managed by
                       ComputeBlurProfile below.
        @param loc the location to look up; The lookup will clamp invalid inputs, but
                   meaningful data are available between 0 and blurred_width
        @param blurred_width The width of the final, blurred rectangle
        @param sharp_width The width of the original, unblurred rectangle.
    */
    static uint8_t ProfileLookup(const uint8_t* profile, int loc, int blurred_width, int sharp_width);

    /** Allocate memory for and populate the profile of a 1D blurred halfplane.  The caller
        must free the memory.  The amount of memory allocated will be exactly 6*sigma bytes.
        @param sigma The standard deviation of the gaussian blur kernel
        @param profile_out The location to store the allocated profile curve
    */

    static void ComputeBlurProfile(SkScalar sigma, uint8_t** profile_out);

    /** Compute an entire scanline of a blurred step function.  This is a 1D helper that
        will produce both the horizontal and vertical profiles of the blurry rectangle.
        @param pixels Location to store the resulting pixel data; allocated and managed by caller
        @param profile Precomputed blur profile computed by ComputeBlurProfile above.
        @param width Size of the pixels array.
        @param sigma Standard deviation of the gaussian blur kernel used to compute the profile;
                     this implicitly gives the size of the pixels array.
    */

    static void ComputeBlurredScanline(uint8_t* pixels, const uint8_t* profile,
                                       unsigned int width, SkScalar sigma);



};

#endif
