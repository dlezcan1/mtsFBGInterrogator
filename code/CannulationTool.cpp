#include "mtsFBGTool/CannulationTool.h"

CMN_IMPLEMENT_SERVICES(CannulationTool);

CannulationTool::CannulationTool(const std::string& filename)
{
    // FIXME: assign tool name
    m_ToolName = "CannulationTool";

    // FIXME: assign base wavelengths
    m_BaseWavelengths.SetSize(3);
    m_BaseWavelengths.Zeros();

    // FIXME: assign calibration matrices
    m_CalibrationMatrixTip.SetSize(2, 3);
    m_CalibrationMatrixTip.SetAll(1.0);
}

CannulationTool::~CannulationTool(){
    
}

mtsDoubleVec CannulationTool::GetForcesTip(const mtsDoubleVec& processedWavelengths)
{
    mtsDoubleVec forces(2);

    forces = m_CalibrationMatrixTip * processedWavelengths;

    return forces;

}

mtsDoubleVec CannulationTool::GetForcesSclera(const mtsDoubleVec& processedWavelengths)
{
    // No Sclera forces implemented
    mtsDoubleVec forces(2);
    forces.Zeros();

    return forces;
}