#ifndef _HYPERION_INTERROGATOR_H
#define _HYPERION_INTERROGATOR_H

#include <cisstCommon.h>

#include "hLibrary.h"

#include "mtsFBGSensor/mtsFBGSensor/Interrogator.h"

class HyperionInterrogator : public Interrogator
{
    public:
        static const unsigned int DEFAULT_PORT = H_CMD_PORT;

        HyperionInterrogator(const std::string& ipAddress, const unsigned int port = DEFAULT_PORT);
        ~HyperionInterrogator();

        // Abstract base class methods
        int GetNumberOfChannels() const override;

        bool Connect() override;
        bool Disconnect() override;

        bool StreamPeaks() override;
        bool DisableStreamPeaks() override;

        // Methods to get the peaks from a channel
        vctDoubleVec GetPeaks() const  override;
        vctDoubleVec GetPeaks(const size_t channelId) const override;

    private: 
        Hyperion* m_Hyperion = nullptr;

        hACQPeaks GetPeaksInterrogator() const;


}; // class; HyperionInterrogator

#endif