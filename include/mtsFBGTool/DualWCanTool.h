#pragma once

#include <cisstMultiTask.h>

#include "FBGToolInterface.h"

class DualWCanTool : public FBGToolInterface
{
    CMN_DECLARE_SERVICES(CMN_NO_DYNAMIC_CREATION, CMN_LOG_LOD_RUN_VERBOSE);

public:

    // initialize with a json file
    DualWCanTool(const std::string& filename);
    virtual ~DualWCanTool();

    virtual mtsDoubleVec GetForcesTip(const mtsDoubleVec& wavelengths);
    virtual mtsDoubleVec GetForcesSclera(const mtsDoubleVec& wavelengths);
    virtual mtsDoubleVec GetMomentSclera(const mtsDoubleVec& wavelengths);


}; // class: DualWCanTool

CMN_DECLARE_SERVICES_INSTANTIATION(DualWCanTool);