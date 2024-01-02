#ifndef _MTSFBGSENSOR_H
#define _MTSFBGSENSOR_H

#include <cisstCommon.h>
#include <cisstMultiTask.h>

#include "mtsFBGSensor/Interrogator.h"

class CISST_EXPORT mtsFBGSensor : public mtsTaskContinuous 
{
    CMN_DECLARE_SERVICES(CMN_NO_DYNAMIC_CREATION, CMN_LOG_LOD_RUN_ERROR);


public: 
    mtsFBGSensor(const std::string& componentName);
    mtsFBGSensor(const mtsTaskContinuousConstructorArg& arg);

    ~mtsFBGSensor();

    void Configure(const std::string & fileName) override;
    void Startup(void) override;
    void Run(void) override;
    void Cleanup(void) override;

    // For mts commands
    inline void GetNumberOfChannels(mtsUInt& number)                        const { number.Data = m_Interrogator->GetNumberOfChannels(); }
    inline void GetNumberOfPeaks(const mtsUInt& channelId, mtsUInt& number) const { number.Data = m_Interrogator->GetNumberOfPeaks(channelId.Data); }

    inline void Connect(mtsBool& success)    { success.Data = m_Interrogator->Connect(); }
    inline void Disconnect(mtsBool& success) { success.Data = m_Interrogator->Disconnect(); }

protected:
    void Init(void);
    void SetupInterfaces(void);

    mtsStateTable m_StateTable;
    mtsDoubleVec  m_Peaks;

private:
    Interrogator* m_Interrogator;


}; // class: mtsFBGSensor

CMN_DECLARE_SERVICES_INSTANTIATION(mtsFBGSensor);


#endif