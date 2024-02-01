#include "mtsFBGSensor/mtsFBGTool/ThreeDOFTool.h"

CMN_IMPLEMENT_SERVICES(ThreeDOFTool);

ThreeDOFTool::ThreeDOFTool(const std::string& filename)
{
    Json::Value   jsonConfig;
    try
    {
        std::ifstream jsonStream;
        Json::Reader  jsonReader;

        jsonStream.open(filename.c_str());

        if (!jsonReader.parse(jsonStream, jsonConfig)) {
            CMN_LOG_CLASS_INIT_ERROR << "Configure ThreeDOFTool"
                                     << ": failed to parse FBG tool configuration file \""
                                     << filename << "\"\n"
                                     << jsonReader.getFormattedErrorMessages();
            return;
        }

        CMN_LOG_CLASS_INIT_VERBOSE << "Configure: ThreeDOFTool"
                                   << " using file \"" << filename << "\"" << std::endl
                                   << "----> content of FBG tool configuration file: " << std::endl
                                   << jsonConfig << std::endl
                                   << "<----" << std::endl;

        // Handle which FBG tool is used
        if (!jsonConfig.isMember("Device_Type"))
        {
            CMN_LOG_CLASS_INIT_ERROR << "Configure ThreeDOFTool"
                                     << ": make sure the configuration file \""
                                     << filename << "\" has the \"Device_Type\" field"
                                     << std::endl;
            return;
        }



    }
    catch(...)
    {
        CMN_LOG_CLASS_INIT_ERROR << "Configure ThreeDOFTool"
                                 << ": make sure the file \""
                                 << filename << "\" is in JSON format"
                                 << std::endl;
    }
    
    // configure tool
    m_ToolName = jsonConfig["Tool_Name"].asString();

    m_BaseWavelengths.DeSerializeTextJSON(jsonConfig["Base_Wavelengths"]);
    m_CalibrationMatrixTip.DeSerializeTextJSON(jsonConfig["Calibration_Matrix_Tip"]);
    
    AssignWavelengthIndicesFromJSON(jsonConfig["Wavelength_Indices_Tip"], m_IndicesTip);

    // handle the polynomial for sclera force calculation
    if (!jsonConfig.isMember("Calibration_Sclera_Polynomial"))
    {
        CMN_LOG_CLASS_INIT_ERROR << "Configure ThreeDOFTool"
                                    << ": make sure the configuration file \""
                                    << filename << "\" has the \"Calibration_Sclera_Polynomial\" field"
                                    << std::endl;
        throw std::runtime_error("\"Calibration_Sclera_Polynomial\" not in JSON configuration file.");
    }
    Json::Value jsonConfigScleraPoly = jsonConfig["Calibration_Sclera_Polynomial"];
    mtsDoubleVec coeffs, boundsMin, boundsMax;
    coeffs.DeSerializeTextJSON(jsonConfigScleraPoly["Coefficients"]);
    boundsMin.DeSerializeTextJSON(jsonConfigScleraPoly["Bounds_Min"]);
    boundsMax.DeSerializeTextJSON(jsonConfigScleraPoly["Bounds_Max"]);

    m_ForceScleraPoly = std::make_unique<BernsteinPolynomial>(2, coeffs, boundsMin, boundsMax);

}
ThreeDOFTool::~ThreeDOFTool(){
    
}

mtsDoubleVec ThreeDOFTool::GetForcesTip(const mtsDoubleVec& processedWavelengths)
{
    mtsDoubleVec wavelengthsTip = PartitionWavelengths(processedWavelengths, m_IndicesTip);

    mtsDoubleVec forcesTip = m_CalibrationMatrixTip * wavelengthsTip;

    return forcesTip;
}

mtsDoubleVec ThreeDOFTool::GetForcesSclera(const mtsDoubleVec& processedWavelengths)
{
    return {m_ForceScleraPoly->Evaluate(processedWavelengths)};
}