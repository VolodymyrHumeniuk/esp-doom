#pragma once
#include "main.h"

// Simple digital limiter
class Limiter
{
    float   envelope    = 0.00000001f; // just starting value
    float   threshold   = 0.05f;
    float   attack      = 0.001f;
    float   release     = 0.005f;

    inline void calcEnvelope( float smp ) {
        float abs_smp = fabsf( smp ); // calc envelope
        if( abs_smp > envelope ) {
            envelope = envelope * ( 1.0f - attack ) + attack * abs_smp;
        } else {
            envelope = envelope * ( 1.0f - release ) + release * abs_smp;
        }
    }

    inline float maxof( const float smp1, const float smp2 ) {
        return smp1 < smp2 ? smp2 : smp1;
    }

public:
    void setThreshold( float th ) { threshold = th; }
    void setAttack( float a ) { attack = a; }
    void setRelease( float r ) { release = r; }

    // Mono limiter
    inline float limit( float smp )
    {
        calcEnvelope( smp );

        if( envelope > threshold ) { // limit
            smp = smp * ( threshold / envelope );
        }

        return smp;
    }

    // Stereo limiter
    inline void stereoLimit( float& smpL, float& smpR )
    {
        // select loudest channel with signal rectification
        float lnk = maxof( fabs( smpL ), fabs( smpR ) );
        
        calcEnvelope( lnk ); // envelope function

        if( envelope > threshold )  // limit
        {
            float k = threshold / envelope;
            smpL *= k;
            smpR *= k;
        }
    }
};