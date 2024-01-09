#pragma once

#include <cisstMultiTask.h>

#include "FBGToolInterface.h"

class DualZeroTool : public FBGToolInterface
{
    CMN_DECLARE_SERVICES(CMN_NO_DYNAMIC_CREATION, CMN_LOG_LOD_RUN_VERBOSE);

public:

    // initialize with a json file
    DualZeroTool(const std::string& filename);
    virtual ~DualZeroTool();

    virtual mtsDoubleVec GetForcesTip(const mtsDoubleVec& wavelengths);
    virtual mtsDoubleVec GetForcesSclera(const mtsDoubleVec& wavelengths);
    virtual mtsDoubleVec GetMomentSclera(const mtsDoubleVec& wavelengths);


protected:
    mtsDoubleVec m_BaseWavelengths;

}; // class: DualZeroTool

CMN_DECLARE_SERVICES_INSTANTIATION(DualZeroTool);