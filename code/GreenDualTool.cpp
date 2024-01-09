#include "mtsFBGTool/GreenDualTool.h"

CMN_IMPLEMENT_SERVICES(GreenDualTool);

GreenDualTool::GreenDualTool(const std::string& filename)
{
    // TODO: load json parameters
    // FIXME: assign tool name
    m_ToolName = "GreenDualTool";
     
    // FIXME
    m_DistanceScleraFBGs = 5.8891; 

    // FIXME: assign base wavelengths
    m_BaseWavelengths.SetSize(9);
    m_BaseWavelengths.Zeros();

    // FIXME: assign calibration matrices
    m_CalibrationMatrixTip.SetSize(2, 3);
    m_CalibrationMatrixTip.SetAll(1.0);

    m_CalibrationMatrixSclera2.SetSize(2, 3);
    m_CalibrationMatrixSclera2.SetAll(1.0);

    m_CalibrationMatrixSclera3.SetSize(2, 3);
    m_CalibrationMatrixSclera3.SetAll(1.0);

    // FIXME: assign tip wavelength convertion matrices
    m_WLConversionTipToSclera2.SetSize(3, 3);
    m_WLConversionTipToSclera2.Zeros();

    m_WLConversionTipToSclera3.SetSize(3,3);
    m_WLConversionTipToSclera3.Zeros();

    // FIXME: assign indices
    m_IndicesTip     = {0, 3, 6};
    m_IndicesSclera2 = {1, 4, 7};
    m_IndicesSclera3 = {2, 5, 8};


} // constructor

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