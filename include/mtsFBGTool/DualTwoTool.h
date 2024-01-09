#pragma once

#include <cisstMultiTask.h>

#include "FBGToolInterface.h"

class DualTwoTool : public FBGToolInterface
{
    CMN_DECLARE_SERVICES(CMN_NO_DYNAMIC_CREATION, CMN_LOG_LOD_RUN_VERBOSE);

public:

    // initialize with a json file
    DualTwoTool(const std::string& filename);
    virtual ~DualTwoTool();

    virtual mtsDoubleVec GetForcesTip(const mtsDoubleVec& wavelengths);
    virtual mtsDoubleVec GetForcesSclera(const mtsDoubleVec& wavelengths);
    virtual mtsDoubleVec GetMomentSclera(const mtsDoubleVec& wavelengths);

}; // class: DualTwoTool

CMN_DECLARE_SERVICES_INSTANTIATION(DualTwoTool);