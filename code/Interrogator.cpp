#include "mtsFBGSensor/mtsFBGSensor/Interrogator.h"

#include "mtsFBGSensor/hyperion/HyperionInterrogator.h"

Interrogator* InterrogatorFactory::CreateInterrogator(const InterrogatorType& type, const std::string& ipAddress, const unsigned int port)
{
    Interrogator* interrogator = nullptr;
    switch (type){
        case InterrogatorType::HYPERION:
            interrogator = new HyperionInterrogator(ipAddress, port);
            break;
        
        default:
            throw std::invalid_argument("Interrogator type is not supported.");

    }

    return interrogator;

}

Interrogator* InterrogatorFactory::CreateInterrogator(const InterrogatorType& type, const std::string& ipAddress)
{
    unsigned int port;
    switch (type){
        case InterrogatorType::HYPERION:
            port = HyperionInterrogator::DEFAULT_PORT;
            break;

        default:
            std::cerr << "Interrogator type not implemented!" << std::endl;
            port = -1;
            break;
    }

    return CreateInterrogator(type, ipAddress, port);

}