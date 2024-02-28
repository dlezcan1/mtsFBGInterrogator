#include "mtsFBGSensor/mtsFBGSensor/mtsFBGSensor.h"

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
    std::string      ipAddress;
    InterrogatorType interrogatorType;

    try
    {
        std::ifstream jsonStream;
        Json::Value   jsonConfig;
        Json::Reader  jsonReader;

        jsonStream.open(fileName.c_str());

        if (!jsonReader.parse(jsonStream, jsonConfig)) {
            CMN_LOG_CLASS_INIT_ERROR << "Configure " << this->GetName()
                                     << ": failed to parse galil controller configuration file \""
                                     << fileName << "\"\n"
                                     << jsonReader.getFormattedErrorMessages();
            return;
        }

        CMN_LOG_CLASS_INIT_VERBOSE << "Configure: " << this->GetName()
                                   << " using file \"" << fileName << "\"" << std::endl
                                   << "----> content of galil controller configuration file: " << std::endl
                                   << jsonConfig << std::endl
                                   << "<----" << std::endl;

        if (!jsonConfig.isMember("IP_Address"))
        {
            CMN_LOG_CLASS_INIT_ERROR << "Configure " << this->GetName()
                                     << ": make sure the configuration file \""
                                     << fileName << "\" has the \"IP_Address\" field"
                                     << std::endl;
            return;
        }
        ipAddress = jsonConfig["IP_Address"].asString();

        if (!jsonConfig.isMember("Interrogator_Type"))
        {
            CMN_LOG_CLASS_INIT_ERROR << "Configure " << this->GetName()
                                     << ": make sure the configuration file \""
                                     << fileName << "\" has the \"Interrogator_Type\" field"
                                     << std::endl;
            return;
        }
        std::string type = jsonConfig["Interrogator_Type"].asString();
        std::transform(
            type.begin(),
            type.end(),
            type.begin(),
            [] (unsigned char c) {return std::toupper(c);}
        );

        if (type == "HYPERION")
            interrogatorType = InterrogatorType::HYPERION;

        else if (type == "SI155")
            interrogatorType = InterrogatorType::SI155;

        else if (type == "SM130")
            interrogatorType = InterrogatorType::SM130;

        else
        {
            CMN_LOG_CLASS_INIT_ERROR << "Configure " << this->GetName()
                                     << ": the configuration file \""
                                     << fileName << "\" has an invalid \"Interrogator_Type\" field: \""
                                     << type << "\""
                                     << std::endl;
        }

    }
    catch(...)
    {
        CMN_LOG_CLASS_INIT_ERROR << "Configure " << this->GetName()
                                 << ": make sure the file \""
                                 << fileName << "\" is in JSON format"
                                 << std::endl;
    }



    m_Interrogator = InterrogatorFactory::CreateInterrogator(
        interrogatorType,
        ipAddress
    );

    if (!m_Interrogator)
    {
        CMN_LOG_CLASS_INIT_ERROR << "Error creating interrogator @ " << ipAddress << std::endl;
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
    mtsInterfaceProvided* intfProvided = this->AddInterfaceProvided("ProvidesFBGSensor");
    if (!intfProvided)
    {
        CMN_LOG_CLASS_INIT_ERROR << "Error adding \"ProvidesFBGSensor\" provided interface \""
                                 << this->GetName()
                                 << "\"!" << std::endl;
        return;
    }

    if (!intfProvided->AddCommandReadState(m_StateTable, m_Peaks, "GetFBGPeaksState"))
    {
        CMN_LOG_CLASS_INIT_ERROR << "Failed to add mtsFBGSensor::GetFBGPeaksState to \""
                                 << intfProvided->GetFullName()
                                 << "\"!" << std::endl;
    }

    intfProvided->AddCommandRead(&mtsFBGSensor::GetNumberOfChannels,       this, "GetNumberOfChannels");
    intfProvided->AddCommandQualifiedRead(&mtsFBGSensor::GetNumberOfPeaks, this, "GetNumberOfPeaks");

    intfProvided->AddCommandVoidReturn(&mtsFBGSensor::Connect,    this, "Connect");
    intfProvided->AddCommandVoidReturn(&mtsFBGSensor::Disconnect, this, "Disonnect");

    delete intfProvided;

}
