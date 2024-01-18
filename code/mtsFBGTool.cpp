#include "mtsFBGSensor/mtsFBGTool/mtsFBGTool.h"

#include <cisstOSAbstraction/osaSleep.h>

#include "mtsFBGSensor/mtsFBGTool/FBGToolFactory.h"

CMN_IMPLEMENT_SERVICES(mtsFBGTool);

mtsFBGTool::mtsFBGTool(const std::string& taskName) : 
    mtsTaskContinuous(taskName, 10000),
    m_StateTable(10000, "FBGTool"),
    m_FilterOneEuroScleraForceX(200, 1.5, 1.0, 1.0),
    m_FilterOneEuroScleraForceY(200, 1.5, 1.0, 1.0)
{
    m_ForcesTipCF.Zeros();
    m_ForcesScleraCF.Zeros();
}

mtsFBGTool::~mtsFBGTool()
{

}

void mtsFBGTool::Configure(const std::string& filename)
{
    // TODO: handle json configuration

    FBGToolDevices device        = FBGToolDevices::GreenDual; // FIXME
    std::string deviceConfigFile = filename;                  // FIXME

    m_FBGTool = FBGToolFactory::GetFBGTool(device, deviceConfigFile);

    // Setup peak container
    size_t numPeaks   = 9;   // FIXME: from deviceConfigFile
    size_t numSamples = 200; // FIXME
    m_WavelengthPeakContainer.Configure(numPeaks, numSamples);
}   

void mtsFBGTool::SetupInterfaces()
{
    // add states to state table
    m_StateTable.AddData(m_Forces,           "MeasuredCartesianForces");

    m_StateTable.AddData(m_ForcesTip,        "MeasuredCartesianForcesTip");
    m_StateTable.AddData(m_ForcesTipNorm,    "MeasuredCartesianForcesTipNorm");

    m_StateTable.AddData(m_ForcesSclera,     "MeasuredCartesianForcesSclera");
    m_StateTable.AddData(m_ForcesScleraNorm, "MeasuredCartesianForcesScleraNorm");

    m_StateTable.AddData(m_ForcesNorm,        "MeasuredCartesianForceNorm");
    m_StateTable.AddData(m_ForcesDirection,   "MeasuredCartesianForceDirection");

    // Add provided interface
    mtsInterfaceProvided * providedInterface = this->AddInterfaceProvided("ProvidesFBGTool");
    if (!providedInterface)
    {
        CMN_LOG_CLASS_INIT_ERROR << "Error adding \"ProvidesFBGTool\" provided interface \"" 
                                 << this->GetName()
                                 << "\"!"
                                 << std::endl;
        return;
    }

    providedInterface->AddCommandReadState(m_StateTable, m_Forces,            "GetMeasuredVectorCartesianForces");
    providedInterface->AddCommandReadState(m_StateTable, m_ForcesTip,         "GetMeasuredVectorCartesianForcesTip");
    providedInterface->AddCommandReadState(m_StateTable, m_ForcesTipNorm,     "GetMeasuredVectorCartesianForcesTipNorm");
    providedInterface->AddCommandReadState(m_StateTable, m_ForcesSclera,      "GetMeasuredVectorCartesianForcesSclera");
    providedInterface->AddCommandReadState(m_StateTable, m_ForcesScleraNorm,  "GetMeasuredVectorCartesianForcesScleraNorm");
    providedInterface->AddCommandReadState(m_StateTable, m_ForcesNorm,        "GetMeasuredVectorCartesianForceNorm");
    providedInterface->AddCommandReadState(m_StateTable, m_ForcesDirection,   "GetMeasuredVectorCartesianForceDirection");

    providedInterface->AddCommandReadState(m_StateTable, m_ForcesTipCF,        "GetMeasuredCartesianForcesTip");
    providedInterface->AddCommandReadState(m_StateTable, m_ForcesScleraCF,     "GetMeasuredCartesianForcesSclera");
    
    providedInterface->AddCommandRead(&FBGToolInterface::GetToolName, m_FBGTool.get(), "GetToolName");

    delete providedInterface;
    
    // Add required interface
    mtsInterfaceRequired* requiredInterface = this->AddInterfaceRequired("RequiresFBGSensor");
    if (!providedInterface)
    {
        CMN_LOG_CLASS_INIT_ERROR << "Error adding \"RequiresFBGSensor\" required interface \"" 
                                 << this->GetName()
                                 << "\"!"
                                 << std::endl;
        return;
    }

    requiredInterface->AddFunction("GetFBGPeaks", m_ReadStateFBGPeaks);

    delete requiredInterface;
}

void mtsFBGTool::Startup()
{

}

void mtsFBGTool::Cleanup()
{
    m_FBGTool.reset();

}

void mtsFBGTool::Run()
{
    ProcessQueuedCommands();
    ProcessQueuedEvents();

    // Get Number of Samples for Peaks 
    //      (Can be updated to update per run for a single peak, rather than all peaks at once)
    mtsDoubleVec peakSample;
    for (size_t i = 0; i < m_WavelengthPeakContainer.NumSamples; i++)
    {
        mtsExecutionResult result = m_ReadStateFBGPeaks(peakSample); // may need to check peak state updated
        if (!result.IsOK())
            continue;

        m_WavelengthPeakContainer.Update(peakSample);
    }

    if (!m_WavelengthPeakContainer.IsFull)
        return;
    
    // Handle forces
    mtsDoubleVec processedPeaks = m_FBGTool->ProcessWavelengthSamples(m_WavelengthPeakContainer.Peaks);
    mtsDouble    timestamp      = mtsTaskManager::GetInstance()->GetTimeServer().GetAbsoluteTimeInSeconds();
    
    
    m_ForcesTip    = m_FBGTool->GetForcesTip(processedPeaks);
    m_ForcesSclera = m_FBGTool->GetForcesSclera(processedPeaks);

    m_ForcesTipNorm    = m_FBGTool->GetForcesTipNorm(m_ForcesTip);
    m_ForcesScleraNorm = m_FBGTool->GetForcesScleraNorm(m_ForcesSclera);

    m_Forces = m_FBGTool->GetForces(processedPeaks);
    m_Forces[m_Forces.size()] = m_FilterOneEuroScleraForceY.filter(
        m_ForcesSclera[1], timestamp
    );

    m_ForcesDirection[0] = atan2(m_Forces[1], m_Forces[0]);
    if (m_Forces.size() > 2)
        m_ForcesDirection[1] = atan2(m_Forces[2], m_Forces[1]);

    // assign wrenches
    m_ForcesTipCF[0] = m_ForcesTip[0];
    m_ForcesTipCF[1] = m_ForcesTip[1];

    m_ForcesScleraCF[0] = m_ForcesSclera[0];
    m_ForcesScleraCF[1] = m_ForcesSclera[1];
}