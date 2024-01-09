#pragma once

#include <cisstMultiTask.h>

#include "FBGToolInterface.h"

class LightPipeTool : public FBGToolInterface
{
    CMN_DECLARE_SERVICES(CMN_NO_DYNAMIC_CREATION, CMN_LOG_LOD_RUN_VERBOSE);

public:

    // initialize with a json file
    LightPipeTool(const std::string& filename);
    virtual ~LightPipeTool();

    virtual mtsDoubleVec GetForcesTip(const mtsDoubleVec& wavelengths);
    virtual mtsDoubleVec GetForcesSclera(const mtsDoubleVec& wavelengths);


}; // class: LightPipeTool

CMN_DECLARE_SERVICES_INSTANTIATION(LightPipeTool);