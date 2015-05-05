#include <assert.h>
#include <stdio.h>
#include <dlib/math.h>
#include "easing.h"

namespace dmEasing
{
    #include "easing_lookup.h"

    const float EASING_SAMPLES_MINUS_ONE_RECIP = (1.0f / (EASING_SAMPLES-1));

    float GetValue(Type type, float t)
    {
        return GetValue(Curve(type), t);
    }

    float GetValue(Curve curve, float t)
    {
        t = dmMath::Clamp(t, 0.0f, 1.0f);
        int sample_count;
        int index1, index2;
        float val1, val2;
        float* lookup;

        if (curve.type == dmEasing::TYPE_FLOAT_VECTOR)
        {
            sample_count = curve.vector->size;
            lookup       = curve.vector->values;
        } else {
            sample_count = EASING_SAMPLES; // 64 samples for built in curves
            lookup       = EASING_LOOKUP;
            lookup      += curve.type * (EASING_SAMPLES + 1); // NOTE: + 1 as the last sample is duplicated
        }

        index1 = (int) (t * (sample_count-1));
        index2 = index1 + 1;
        if (index2 > sample_count-1)
        {
            index2 = index1;
        }

        val1 = lookup[index1];
        val2 = lookup[index2];

        float diff = (t - index1 * (1.0f / (sample_count-1))) * (sample_count-1);
        return val1 * (1.0f - diff) + val2 * diff;
    }
}

