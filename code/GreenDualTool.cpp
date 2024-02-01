#include "mtsFBGSensor/mtsFBGTool/GreenDualTool.h"

CMN_IMPLEMENT_SERVICES(GreenDualTool);

GreenDualTool::GreenDualTool(const std::string& filename)
{
    Json::Value   jsonConfig;
    try
    {
        std::ifstream jsonStream;
        Json::Reader  jsonReader;

        jsonStream.open(filename.c_str());

        if (!jsonReader.parse(jsonStream, jsonConfig)) {
            CMN_LOG_CLASS_INIT_ERROR << "Configure GreenDualTool"
                                     << ": failed to parse FBG tool configuration file \""
                                     << filename << "\"\n"
                                     << jsonReader.getFormattedErrorMessages();
            return;
        }

        CMN_LOG_CLASS_INIT_VERBOSE << "Configure: GreenDualTool"
                                   << " using file \"" << filename << "\"" << std::endl
                                   << "----> content of FBG tool configuration file: " << std::endl
                                   << jsonConfig << std::endl
                                   << "<----" << std::endl;

        // Handle which FBG tool is used
        if (!jsonConfig.isMember("Device_Type"))
        {
            CMN_LOG_CLASS_INIT_ERROR << "Configure GreenDualTool"
                                     << ": make sure the configuration file \""
                                     << filename << "\" has the \"Device_Type\" field"
                                     << std::endl;
            return;
        }



    }
    catch(...)
    {
        CMN_LOG_CLASS_INIT_ERROR << "Configure GreenDualTool"
                                 << ": make sure the file \""
                                 << filename << "\" is in JSON format"
                                 << std::endl;
    }
    
    // configure tool
    m_ToolName           = jsonConfig["Tool_Name"].asString();
    m_DistanceScleraFBGs = jsonConfig["Distance_Sclera_FBG"].asDouble();

    m_BaseWavelengths.DeSerializeTextJSON(jsonConfig["Base_Wavelengths"]);
    m_CalibrationMatrixTip.DeSerializeTextJSON(jsonConfig["Calibration_Matrix_Tip"]);
    m_CalibrationMatrixSclera2.DeSerializeTextJSON(jsonConfig["Calibration_Matrix_Sclera2"]);
    m_CalibrationMatrixSclera3.DeSerializeTextJSON(jsonConfig["Calibration_Matrix_Sclera3"]);

    AssignWavelengthIndicesFromJSON(jsonConfig["Wavelength_Indices_Tip"],     m_IndicesTip);
    AssignWavelengthIndicesFromJSON(jsonConfig["Wavelength_Indices_Sclera2"], m_IndicesSclera2);
    AssignWavelengthIndicesFromJSON(jsonConfig["Wavelength_Indices_Sclera3"], m_IndicesSclera3);
    
} // constructor

GreenDualTool::~GreenDualTool(){
    
}

mtsDoubleVec GreenDualTool::GetForcesTip(const mtsDoubleVec& wavelengths)
{   
    // grab the tip wavelengths
    mtsDoubleVec wavelengthsTip = PartitionWavelengths(wavelengths, m_IndicesTip);

    return m_CalibrationMatrixTip * wavelengthsTip;
}

mtsDoubleVec GreenDualTool::GetForcesSclera(const mtsDoubleVec& wavelengths)
{
    // partition wavelengths
    mtsDoubleVec wavelengthsTip     = PartitionWavelengths(wavelengths, m_IndicesTip);
    mtsDoubleVec wavelengthsSclera2 = PartitionWavelengths(wavelengths, m_IndicesSclera2);
    mtsDoubleVec wavelengthsSclera3 = PartitionWavelengths(wavelengths, m_IndicesSclera2);

    // calculate moments
    mtsDoubleVec momentSclera2 = m_CalibrationMatrixSclera2 * (wavelengthsSclera2 - m_WLConversionTipToSclera2 * wavelengthsTip);
    mtsDoubleVec momentSclera3 = m_CalibrationMatrixSclera3 * (wavelengthsSclera3 - m_WLConversionTipToSclera3 * wavelengthsTip);
    
    mtsDoubleVec forcesSclera;
    forcesSclera     = (momentSclera3 - momentSclera2).Divide(m_DistanceScleraFBGs);
    forcesSclera[1] *= -1; //rectify Fs-y direction, make the tool force frame align with robot base frame
    
}

double GreenDualTool::GetForcesTipNorm(const mtsDoubleVec& forces) const
{
    return (forces.Norm() - 0.4773) / 1.1569;
}