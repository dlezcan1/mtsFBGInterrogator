#include "hyperion/HyperionInterrogator.h"

#include <cisstCommon/cmnLogger.h>

HyperionInterrogator::HyperionInterrogator(const std::string& ipAddress, const unsigned int port) :
    Interrogator(ipAddress, port)
{

}

HyperionInterrogator::~HyperionInterrogator()
{
    if (!m_Hyperion)
        return;

    Disconnect();
    delete m_Hyperion;

}

bool HyperionInterrogator::Connect()
{
    try
    {
        if (!m_Hyperion) 
            m_Hyperion = new Hyperion(m_IpAddress, m_Port);

        m_Hyperion->connect_comm();

    }
    catch(const std::exception& e)
    {
        std::cerr << "Error connecting Hyperion Interrogator @ " << m_IpAddress << std::endl;
        std::cerr << e.what() << std::endl;

        return false;
    }

    return true;
}

bool HyperionInterrogator::Disconnect()
{
    if (m_Hyperion)
    {
        DisableStreamPeaks();
        m_Hyperion->close_comm();
    }

    return true;
}

int HyperionInterrogator::GetNumberOfChannels() const {
    hACQPeaks peaksMsg = m_Hyperion->get_peaks();

    int numChannels = 0;

    for (int chIdx = 1; chIdx <= H_MAX_NUM_CHANNELS; chIdx++)
        if (peaksMsg.get_channel(chIdx).size() > 0) // count only channels with peaks presented
            numChannels++;


    return numChannels;

}

vctDoubleVec HyperionInterrogator::GetPeaks() const
{
    hACQPeaks peaksMsg    = GetPeaksInterrogator();
    auto peaksAll         = peaksMsg.get_all();
    vctDoubleVec peaksVct = vctDoubleVec(peaksAll.size());
    
    for (size_t i = 0; i < peaksVct.size(); i++)
        peaksVct[i] = peaksAll[i];

    return peaksVct;
}

vctDoubleVec HyperionInterrogator::GetPeaks(const size_t channelId) const
{
    hACQPeaks peaksMsg    = GetPeaksInterrogator();
    auto peaksAll         = peaksMsg.get_channel(channelId);
    vctDoubleVec peaksVct = vctDoubleVec(peaksAll.size());
    
    for (size_t i = 0; i < peaksVct.size(); i++)
        peaksVct[i] = peaksAll[i];

    return peaksVct;
}

hACQPeaks HyperionInterrogator::GetPeaksInterrogator() const 
{
    if (m_isStreaming)
        return m_Hyperion->stream_peaks();

    return m_Hyperion->get_peaks();
}

bool HyperionInterrogator::DisableStreamPeaks()
{
    m_Hyperion->disable_peak_streaming();
    m_isStreaming = false;

    return !GetIsStreaming();
}

bool HyperionInterrogator::StreamPeaks()
{
    int streamingDivider = 1;
    m_Hyperion->enable_peak_streaming(streamingDivider);
    m_isStreaming = true;

    m_Hyperion->stream_peaks(); // clear the buffer

    return GetIsStreaming();
}