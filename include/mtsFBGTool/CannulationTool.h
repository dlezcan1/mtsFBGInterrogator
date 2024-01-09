#pragma once

#include <cisstMultiTask.h>

#include "FBGToolInterface.h"

class CannulationTool : public FBGToolInterface
{
    CMN_DECLARE_SERVICES(CMN_NO_DYNAMIC_CREATION, CMN_LOG_LOD_RUN_VERBOSE);

public:

    // initialize with a json file
    CannulationTool(const std::string& filename);
    virtual ~CannulationTool();

    virtual mtsDoubleVec GetForcesTip(const mtsDoubleVec& wavelengths);
    virtual mtsDoubleVec GetForcesSclera(const mtsDoubleVec& wavelengths);
    virtual mtsDoubleVec GetMomentSclera(const mtsDoubleVec& wavelengths);
    

}; // class: CannulationTool

CMN_DECLARE_SERVICES_INSTANTIATION(CannulationTool);