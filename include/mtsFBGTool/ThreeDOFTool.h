#pragma once

#include <vector>
#include <cisstMultiTask.h>
#include <cisstNumerical/nmrBernsteinPolynomial.h>

#include "FBGToolInterface.h"
#include "UtilMath/BernsteinPolynomial.h"

class ThreeDOFTool : public FBGToolInterface
{
    CMN_DECLARE_SERVICES(CMN_NO_DYNAMIC_CREATION, CMN_LOG_LOD_RUN_VERBOSE);

public:

    // initialize with a json file
    ThreeDOFTool(const std::string& filename);
    virtual ~ThreeDOFTool();

    virtual mtsDoubleVec GetForcesTip(const mtsDoubleVec&    processedWavelengths);
    virtual mtsDoubleVec GetForcesSclera(const mtsDoubleVec& processedWavelengths);

protected:
    BernsteinPolynomial m_ForceScleraPoly;
    mtsDoubleMat        m_CalibrationMatrixTip;
    std::vector<size_t> m_IndicesTip;

}; // class: ThreeDOFTool

CMN_DECLARE_SERVICES_INSTANTIATION(ThreeDOFTool);