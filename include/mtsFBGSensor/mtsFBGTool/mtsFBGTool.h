#pragma once
#include <memory>

#include <cisstMultiTask.h>

#include "FBGToolInterface.h"
#include "SensorFilters/SensorOneEuroFilter.h"

class mtsFBGTool : public mtsTaskContinuous
{
    CMN_DECLARE_SERVICES(CMN_NO_DYNAMIC_CREATION, CMN_LOG_LOD_RUN_VERBOSE);
public:
    
    mtsFBGTool(const std::string& taskName);
    ~mtsFBGTool();
    
    void Configure(const std::string & filename);
    void Startup(void);
    void Run(void);
    void Cleanup(void);

protected:
    void SetupInterfaces(void);

private:
    mtsStateTable                     m_StateTable;
    std::shared_ptr<FBGToolInterface> m_FBGTool;

    // Container to handle number of peaks
    struct {
        size_t       NumSamples;
        mtsDoubleMat Peaks;
        size_t       CurrentIndex;
        bool         IsFull = false;

        void Configure(size_t numPeakSignals, size_t numSamples = 0)
        {
            if (numSamples > 0)
                NumSamples = NumSamples;
            
            Peaks.SetSize(NumSamples, numPeakSignals);
            Peaks.Zeros();
        }

        void Update(const mtsDoubleVec& peaks)
        {
            Peaks[CurrentIndex++] = peaks;
            if (CurrentIndex >= NumSamples)
            {
                IsFull        = true; 
                CurrentIndex %= NumSamples;
            }
        }

        void Reset()
        {
            CurrentIndex = 0;
            IsFull       = false;
        }

    } m_WavelengthPeakContainer;

    // Member States
    mtsDoubleVec m_Forces;
    mtsDoubleVec m_ForcesTip;
    mtsDoubleVec m_ForcesSclera;
    
    mtsDouble    m_ForcesNorm;
    mtsDouble    m_ForcesTipNorm;
    mtsDouble    m_ForcesScleraNorm;
    
    mtsDoubleVec m_ForcesDirection;

    // Member functions
    mtsFunctionRead m_ReadStateFBGPeaks;

    // Sensor Filters
    sensorOneEuroFilter m_FilterOneEuroScleraForceX;
    sensorOneEuroFilter m_FilterOneEuroScleraForceY;

}; // class: mtsFBGTool

CMN_DECLARE_SERVICES_INSTANTIATION(mtsFBGTool);