#pragma once

#include <cisstMultiTask.h>

#include "FBGToolInterface.h"

class ThreeDOFTool : public FBGToolInterface
{
    CMN_DECLARE_SERVICES(CMN_NO_DYNAMIC_CREATION, CMN_LOG_LOD_RUN_VERBOSE);

public:

    // initialize with a json file
    ThreeDOFTool(const std::string& filename);
    virtual ~ThreeDOFTool();

    virtual mtsDoubleVec GetForcesTip(const mtsDoubleVec& wavelengths);
    virtual mtsDoubleVec GetForcesSclera(const mtsDoubleVec& wavelengths);
    virtual mtsDoubleVec GetMomentSclera(const mtsDoubleVec& wavelengths);


}; // class: ThreeDOFTool

CMN_DECLARE_SERVICES_INSTANTIATION(ThreeDOFTool);