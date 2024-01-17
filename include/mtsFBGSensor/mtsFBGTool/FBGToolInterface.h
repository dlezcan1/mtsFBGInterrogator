#pragma once

#include <cisstCommon.h>
#include <cisstMultiTask.h>
#include <cisstVector/vctDynamicVector.h>

class FBGToolInterface : public cmnGenericObject
{
    CMN_DECLARE_SERVICES(CMN_NO_DYNAMIC_CREATION, CMN_LOG_LOD_RUN_VERBOSE);

public:
    virtual ~FBGToolInterface(){}

    inline virtual mtsStdString GetToolName()               const { return m_ToolName; }
    inline virtual void GetToolName(mtsStdString& toolName) const {  toolName = m_ToolName; }
    
    virtual mtsDoubleVec GetForces(const mtsDoubleVec& processedWavelengths) { 
        mtsDoubleVec forcesTip    = GetForcesTip(processedWavelengths);
        mtsDoubleVec forcesSclera = GetForcesSclera(processedWavelengths);

        return mtsDoubleVec({
            forcesTip[0], 
            forcesTip[1], 
            GetForcesScleraNorm(forcesSclera)
        });
    }
    virtual mtsDoubleVec GetForcesTip(const mtsDoubleVec& processedWavelengths)    = 0;
    virtual mtsDoubleVec GetForcesSclera(const mtsDoubleVec& processedWavelengths) = 0;
    
    virtual double       GetForcesNorm(const mtsDoubleVec& force)       const { return force.Norm(); }
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

protected: 
    mtsStdString m_ToolName;
    mtsDoubleVec m_BaseWavelengths;

    static mtsDoubleVec PartitionWavelengths(const mtsDoubleVec& wavelengths, const std::vector<size_t>& indicesMap){
        mtsDoubleVec partitionedWavelengths(indicesMap.size());
        for (size_t i = 0; i < indicesMap.size(); i++)
        {
            partitionedWavelengths[i] = wavelengths[indicesMap[i]];
        }

        return partitionedWavelengths;
    }

}; // abstract class: FBGToolInterface