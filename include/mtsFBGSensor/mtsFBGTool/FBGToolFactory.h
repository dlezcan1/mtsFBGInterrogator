#pragma once

#include <memory>

#include "FBGToolInterface.h"

#include "GreenDualTool.h"
#include "CannulationTool.h"
#include "ThreeDOFTool.h"

enum FBGToolDevices {
    Cannulation,
    GreenDual,
    ThreeDOF,

}; // enum: FBGToolDevices

class FBGToolFactory 
{
public:
    static std::shared_ptr<FBGToolInterface> GetFBGTool(
        const FBGToolDevices& device,
        const std::string& configFileName
    )
    {
        switch (device)
        {
            case FBGToolDevices::GreenDual:
                return std::make_shared<GreenDualTool>(configFileName);

            case FBGToolDevices::Cannulation:
                return std::make_shared<CannulationTool>(configFileName);

            case FBGToolDevices::ThreeDOF:
                return std::make_shared<ThreeDOFTool>(configFileName);

            default:
                throw std::invalid_argument("FBG Tool device not supported!");
        }
    }
}; // FBGToolFactory