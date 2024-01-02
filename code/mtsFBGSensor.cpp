#include "mtsFBGSensor/mtsFBGSensor.h"

CMN_IMPLEMENT_SERVICES_DERIVED(mtsFBGSensor, mtsTaskContinuous);

mtsFBGSensor::mtsFBGSensor(const std::string& componentName) : mtsTaskContinuous(componentName), m_StateTable(50, "FBGSensorState")
{
    SetupInterfaces();
}

mtsFBGSensor::mtsFBGSensor(const mtsTaskContinuousConstructorArg& arg) : mtsTaskContinuous(arg), m_StateTable(50, "FBGSensorlState")
{
    SetupInterfaces();
}

mtsFBGSensor::~mtsFBGSensor() 
{
    if (m_Interrogator)
        delete m_Interrogator;
}


void mtsFBGSensor::Configure(const std::string & fileName)
{
    std::string ipAddress = fileName;
    
    m_Interrogator = InterrogatorFactory::CreateInterrogator(
        InterrogatorType::HYPERION, // FIXME: change to config from json file
        ipAddress                   // FIXME: change to config from json file
    );

    if (!m_Interrogator)
    {
        CMN_LOG_CLASS_INIT_ERROR << "Error creating Hyperion interrogator @" << ipAddress << std::endl;
    }

    m_Interrogator->StreamPeaks(); // FIXME: change to config from json file

}

void mtsFBGSensor::Startup()
{
    if (!m_Interrogator->Connect())
    {
        CMN_LOG_CLASS_INIT_ERROR << "Error connecting to interrogator!" << std::endl;
    }
}

void mtsFBGSensor::Run()
{
    this->ProcessQueuedCommands();
    this->ProcessQueuedEvents();

    if (!m_Interrogator)
        return;

    // get the current interrogator peaks
    m_Peaks = m_Interrogator->GetPeaks();

}

void mtsFBGSensor::Cleanup()
{
    if (!m_Interrogator)
        return;

    m_Interrogator->Disconnect();
    
}

void mtsFBGSensor::SetupInterfaces()
{
    m_StateTable.AddData(m_Peaks, "Peaks");

    // Add the interface
    mtsInterfaceProvided* intfProvided = this->AddInterfaceProvided("FBGSensor");
    if (!intfProvided)
    {
        CMN_LOG_CLASS_INIT_ERROR << "Error adding \"FBGSensor\" provided interface \"" 
                                 << this->GetName()
                                 << "\"!" << std::endl;
        return;
    }

    if (!intfProvided->AddCommandReadState(m_StateTable, m_Peaks, "GetPeaksState"))
    {
        CMN_LOG_CLASS_INIT_ERROR << "Failed to add mtsFBGSensor::GetPeaksState to \""
                                 << intfProvided->GetFullName()
                                 << "\"!" << std::endl;
    }

    intfProvided->AddCommandRead(&mtsFBGSensor::GetNumberOfChannels,       this, "GetNumberOfChannels");
    intfProvided->AddCommandQualifiedRead(&mtsFBGSensor::GetNumberOfPeaks, this, "GetNumberofPeaks");

    intfProvided->AddCommandVoidReturn(&mtsFBGSensor::Connect,    this, "Connect");
    intfProvided->AddCommandVoidReturn(&mtsFBGSensor::Disconnect, this, "Disonnect");

    delete intfProvided;

}
