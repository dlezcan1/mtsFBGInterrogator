#include "mtsFBGTool/ThreeDOFTool.h"

CMN_IMPLEMENT_SERVICES(ThreeDOFTool);

ThreeDOFTool::ThreeDOFTool(const std::string& filename)
{
    m_ToolName = "ThreeDOFTool";

    // TODO: update json
    mtsDoubleVec coeffs, boundsMin, boundsMax;
    coeffs.Assign(
        343.607,    21.867,     941.907,    -1391.959,  -11.469,    -961.724,
        1768.472,   -410.569,   684.794,    -712.421,   -467.469,   -1091.697,
        3277.402,   -650.449,   1369.105,   -5922.652,  2270.995,   -1416.751,
        506.425,    312.150,    600.015,    -4143.529,  1628.381,   -1171.994,
        6679.001,   -3204.730,  1612.565,   93.334,     516.164,    -2572.898,
        -66.831,    -490.031,   2248.976,   -2531.977,  1431.287,   -1319.843,
        172.108,    -700.485,   2935.791,   -2179.248,  1380.439,   -2876.423,
        6512.275,   -3547.797,  2465.253,   -2136.571,  1503.850,   -1621.256,
        4926.398,   -3283.491,  2368.686,   -8051.867,  4259.618,   -2249.076,
        590.997,    -734.489,   2368.295,   -1444.211,  1241.521,   -1610.553,
        2948.673,   -1405.732,  958.130,    -1557.888,  1465.675,   -2151.500,
        3360.556,   -2371.984,  2558.014,   -5132.822,  2669.627,   -1559.169,
        2818.355,   -1429.369,  1024.950,   -4672.584,  2654.343,   -1512.145,
        3905.311,   -2173.105,  1128.373
    ); // FIXME
    boundsMin.Assign(-0.25751, -0.24409,   -0.22805,   -0.23807); // FIXME
    boundsMax.Assign( 0.25736,  0.24370,    0.23070,    0.10977); // FIXME
    
    m_ForceScleraPoly = std::make_unique<BernsteinPolynomial>(2,coeffs, boundsMin, boundsMax);

    m_IndicesTip = {0, 1, 2}; // FIXME

    m_CalibrationMatrixTip.SetSize(2, m_IndicesTip.size());
    m_CalibrationMatrixTip.Assign(   
        83.408,   -40.409,    -42.999,
        -3.979,   -78.642,     82.622
    ); // FIXME


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