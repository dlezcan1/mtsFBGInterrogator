#pragma once

#include <cisstCommon.h>
#include <cisstMultiTask.h>
#include <cisstVector/vctDynamicVector.h>

class FBGToolInterface : public cmnGenericObject
{
    CMN_DECLARE_SERVICES(CMN_NO_DYNAMIC_CREATION, CMN_LOG_LOD_RUN_VERBOSE);

public:
    virtual ~FBGToolInterface(){}

    virtual mtsDoubleVec GetForcesTip(const mtsDoubleVec& processedWavelengths)    = 0;
    virtual mtsDoubleVec GetForcesSclera(const mtsDoubleVec& processedWavelengths) = 0;
    
    virtual double       GetForcesTipNorm(const mtsDoubleVec& force)    const { return force.Norm(); }
    virtual double       GetForcesScleraNorm(const mtsDoubleVec& force) const { return force.Norm(); }

    mtsDoubleVec        GetBaseWavelengths(void) const { return m_BaseWavelengths; }
    inline virtual void SetBaseWavelengths(const mtsDoubleVec& wavelengths) { m_BaseWavelengths = wavelengths; }

    mtsDoubleVec ProcessWavelengthSamples(const mtsDoubleMat& wavelengths)
    {
        mtsDoubleVec processedWavelengths;
        processedWavelengths.SetSize(m_BaseWavelengths.size());
        processedWavelengths.SetAll(0);

        size_t N_rows = wavelengths.rows();
        for (size_t i=0; i < N_rows; i++)
        {
            processedWavelengths += ProcessWavelengthSamples(wavelengths[i]);
        }

        return processedWavelengths / ((double) N_rows);
    }

    virtual mtsDoubleVec ProcessWavelengthSamples(const mtsDoubleVec& wavelengths)
    {
        return wavelengths - m_BaseWavelengths;
    }

    virtual mtsDoubleVec ProcessWavelengthSamples(const vctDynamicVector<double>& wavelengths)
    {
        return ProcessWavelengthSamples(mtsDoubleVec(wavelengths));
    }


    mtsDoubleVec m_BaseWavelengths;

}; // abstract class: FBGToolInterface