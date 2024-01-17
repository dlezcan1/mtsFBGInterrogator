#ifndef _INTERROGATOR_H
#define _INTERROGATOR_H

#include <cisstCommon.h>
#include <cisstVector.h>

enum InterrogatorType{
    HYPERION,
    SI155 = HYPERION,
    SM130,
}; // enum: InterrogatorType


class CISST_EXPORT Interrogator
{
public:
    Interrogator(const std::string& ipAddress, const unsigned int port) : m_IpAddress(ipAddress), m_Port(port) {}
    Interrogator(const Interrogator& interrogator) = default;
    virtual ~Interrogator(){} 


    // Abstract base class methods
    inline int  GetNumberOfPeaks(const size_t channelId) const { return this->GetPeaks(channelId).size(); }
    virtual int GetNumberOfChannels() const = 0;

    virtual bool Connect()    = 0;
    virtual bool Disconnect() = 0;

    virtual inline bool GetIsStreaming() { return m_isStreaming; }
    virtual bool StreamPeaks()           { return GetIsStreaming(); }
    virtual bool DisableStreamPeaks()    { return !GetIsStreaming(); }

    // Methods to get the peaks from a channel
    virtual vctDoubleVec GetPeaks(const size_t channelId) const = 0;
    virtual vctDoubleVec GetPeaks() const
    {
        vctDoubleVec peaks = vctDoubleVec();
        size_t idx = 0;
        for (int chId = 1; chId <= this->GetNumberOfChannels(); chId++)
        {
            auto peaks_chID = this->GetPeaks(chId);
            peaks.SetSize(peaks.size() + peaks_chID.size());
            for (auto peak : peaks_chID)
                peaks[idx++] = peak;

        }

        return peaks;
    }

    protected:
        std::string  m_IpAddress;
        unsigned int m_Port;
        bool         m_isStreaming = false;

}; // class: Interrogator

class InterrogatorFactory
{
    public:
        static Interrogator* CreateInterrogator(const InterrogatorType& type, const std::string& ipAddress);
        static Interrogator* CreateInterrogator(const InterrogatorType& type, const std::string& ipAddress, const unsigned int port);

}; // class: InterrogatorFactory


#endif