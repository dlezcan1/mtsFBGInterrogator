#include "mtsFBGSensor/mtsFBGTool/CannulationTool.h"

CMN_IMPLEMENT_SERVICES(CannulationTool);

CannulationTool::CannulationTool(const std::string& filename)
{
    Json::Value   jsonConfig;
    try
    {
        std::ifstream jsonStream;
        Json::Reader  jsonReader;

        jsonStream.open(filename.c_str());

        if (!jsonReader.parse(jsonStream, jsonConfig)) {
            CMN_LOG_CLASS_INIT_ERROR << "Configure CannulationTool"
                                     << ": failed to parse FBG tool configuration file \""
                                     << filename << "\"\n"
                                     << jsonReader.getFormattedErrorMessages();
            return;
        }

        CMN_LOG_CLASS_INIT_VERBOSE << "Configure: CannulationTool"
                                   << " using file \"" << filename << "\"" << std::endl
                                   << "----> content of FBG tool configuration file: " << std::endl
                                   << jsonConfig << std::endl
                                   << "<----" << std::endl;

        // Handle which FBG tool is used
        if (!jsonConfig.isMember("Device_Type"))
        {
            CMN_LOG_CLASS_INIT_ERROR << "Configure CannulationTool"
                                     << ": make sure the configuration file \""
                                     << filename << "\" has the \"Device_Type\" field"
                                     << std::endl;
            return;
        }



    }
    catch(...)
    {
        CMN_LOG_CLASS_INIT_ERROR << "Configure CannulationTool"
                                 << ": make sure the file \""
                                 << filename << "\" is in JSON format"
                                 << std::endl;
    }
    
    // configure tool
    m_ToolName = jsonConfig["Tool_Name"].asString();

    m_BaseWavelengths.DeSerializeTextJSON(jsonConfig["Base_Wavelengths"]);
    m_CalibrationMatrixTip.DeSerializeTextJSON(jsonConfig["Calibration_Matrix_Tip"]);
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