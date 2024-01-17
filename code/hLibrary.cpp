//
//  hLibrary.cpp
//  hLibrary
//
//  Created by Dustin Carr on 8/6/14.
//  Copyright (c) 2014 Micron Optics, Inc. All rights reserved.
//





#include <iostream>


#ifdef _WIN32
//Windows
/* Link against Ws2_32.lib for Windows */
#include <winsock2.h>
#include <WS2tcpip.h>
#include <io.h>
#include <cstdint>
#include <time.h>
#include <algorithm>

#elif defined __linux__
//Linux
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#elif defined __APPLE__
//OS X
#include <sys/socket.h>
#include <sys/time.h>
#include <cmath>
#include <arpa/inet.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#endif

#include "mtsFBGSensor/hyperion/hLibrary.h"

#ifdef _WIN32
void sleep(double seconds)
{
	time_t start, current;
	time(&start);
	do
	{
		time(&current);
	} while (difftime(current, start) < seconds);
}
#endif




//------------------------------------------------------------------------------------
//
//                              Hyperion methods
//
//------------------------------------------------------------------------------------



Hyperion::Hyperion(hComm* comm): comm(comm)
{
    init();
}

Hyperion::Hyperion(std::string ipAddress, int port, int timeout)
{
    comm = new hCommTCPSocket(ipAddress, port, timeout);
    connect_comm();
    init();
}

Hyperion::~Hyperion()
{
    try
    {
        delete comm;
    }
    catch (...)
    {
        
    }
    if(peakContent != nullptr)
    {
        delete peakContent;
    }
    
    if(spectrumContent != nullptr)
    {
        delete spectrumContent;
    }
    
    if(peakStreamComm != nullptr)
    {
        delete peakStreamComm;
    }
    
    if(spectrumStreamComm != nullptr)
    {
        delete spectrumStreamComm;
    }
    
    
    
}

void Hyperion::init()
{

    peakStreamComm = nullptr;
    spectrumStreamComm = nullptr;
    peakContent = nullptr;
    spectrumContent = nullptr;
    calibrationOffset.clear();
    calibrationScale.clear();
    calibrationInvScale.clear();
    
    get_power_calibration_info(calibrationOffset, calibrationScale);
    
    for(int scale:calibrationScale)
    {
        calibrationInvScale.push_back(1.0/double(scale));
    }
    
    
    spectrum = get_raw_spectrum(-1);
    
    startWvl = spectrum.spectrumHeader->startWavelength;
    deltaWvl = spectrum.spectrumHeader->wavelengthIncrement;
    numWvl = spectrum.spectrumHeader->numPoints;
    numChannels = spectrum.spectrumHeader->numChannels;
        
}

void Hyperion::copy_response_content(uint8_t *copyBuffer)
{
    memcpy(copyBuffer, lastResponse.content, lastResponse.contentLength);
}

void Hyperion::get_user_data(int slot, uint8_t* userDataBuffer)
{
    lastResponse = comm->execute_command("#GetUserData", std::to_string(slot), hREQUEST_OPT_SUPPRESS_MSG);
    copy_response_content(userDataBuffer);
}


std::string Hyperion::get_serial_number()
{

    lastResponse = comm->execute_command("#GetSerialNumber", "", hREQUEST_OPT_NONE);
    std::string serialNumber = std::string((char*)lastResponse.content, lastResponse.contentLength);
    return serialNumber;
    
}

std::string Hyperion::get_library_version()
{
    return std::string(H_LIB_VERSION);
}

void Hyperion::get_version(std::string &fwVersion, std::string &fPGAVersion, std::string &hLibraryVersion)
{
    
    hLibraryVersion = std::string(H_LIB_VERSION);
    
    lastResponse = comm->execute_command("#GetFirmwareVersion", "", hREQUEST_OPT_NONE);
    
    fwVersion = std::string((char *)lastResponse.content, lastResponse.contentLength);
    
    lastResponse = comm->execute_command("#GetFpgaVersion", "", hREQUEST_OPT_NONE);
    
    fPGAVersion = std::string((char *)lastResponse.content, lastResponse.contentLength);
    
}


void Hyperion::set_instrument_name(std::string instrumentName)
{
    
    lastResponse = comm->execute_command("#SetInstrumentName", instrumentName, hREQUEST_OPT_NONE);
    
}

std::string Hyperion::get_instrument_name()
{
    lastResponse = comm->execute_command("#GetInstrumentName", "", hREQUEST_OPT_NONE);
    
    return std::string((char *)lastResponse.content, lastResponse.contentLength);

}

const hACQPeaks Hyperion::get_peaks()
{
    double * peakData;
    
    lastResponse = comm->execute_command("#GetPeaks", "", hREQUEST_OPT_SUPPRESS_MSG);
    
    if(peakContent != nullptr)
    {
        delete peakContent;
    }
    
    peakContent = new uint8_t[lastResponse.contentLength];
    
    copy_response_content(peakContent);
    peaksHeader = (hACQPeaksHeader*)peakContent;
    peakData = (double *)(peakContent + sizeof(hACQPeaksHeader));
    return  hACQPeaks(peaksHeader->peakCounts, peakData, *peaksHeader);

}

void Hyperion::get_scan_parameters(double &startWavelength, double &deltaWavelength, int &numWavelengths)
{
    
    startWavelength = startWvl;
    deltaWavelength = deltaWvl;
    numWavelengths = numWvl;

}

const hACQSpectrum Hyperion::get_spectrum(int numChannelsToReturn, int* channelsToReturn)
{
    
    int channel;
    uint16_t* currentSpectrum;
    int outputSpectrumIndex = 0;
    std::vector<double> outputSpectrum;
    //Return all channels if numChannels == 0.
    if(numChannelsToReturn == 0)
    {
        numChannelsToReturn = numChannels;
    }
    //Stream the spectrum if possible
    if(spectrumStreamComm != nullptr && spectrumStreamComm->is_connected())
    {
        spectrum = stream_spectrum();
    }
    else if(numChannelsToReturn == 1)
    {
        spectrum = get_raw_spectrum(channelsToReturn[0]);
    }
    else
    {
        spectrum = get_raw_spectrum(-1);
    }
    
    
    outputSpectrum.resize(numChannelsToReturn*spectrum.spectrumHeader->numPoints);
    
    for(int channelIndex = 0; channelIndex < numChannelsToReturn; channelIndex++)
    {
        //Return all channels if channelsToReturn is null
        if (channelsToReturn)
        {
            channel = channelsToReturn[channelIndex] - 1;
        }
        else
        {
            channel = channelIndex;
        }
        int currentSpectrumBase = channel*spectrum.spectrumHeader->numPoints;
        for(int spectrumIndex = 0; spectrumIndex < spectrum.spectrumHeader->numPoints; spectrumIndex++)
        {
            outputSpectrum[outputSpectrumIndex++] = double(spectrum.rawSpectrumData[currentSpectrumBase + spectrumIndex])*
                calibrationInvScale[channel] + calibrationOffset[channel];
        }
    }

    spectrum.calibratedSpectrumData.swap(outputSpectrum);
    
    return spectrum;
}

const hACQSpectrum Hyperion::get_raw_spectrum(int channel)
{
    
    uint16_t* spectrumStart;
    uint16_t* spectrumEnd;
    hACQSpectrum rawSpectrum;
    
    std::string argument;
    if (channel == -1)
    {
        argument = "";
    }
    else
    {
        argument = std::to_string(channel);
    }
    
    lastResponse = comm->execute_command("#GetSpectrum", argument, hREQUEST_OPT_SUPPRESS_MSG);
    
    if(spectrumContent != nullptr)
    {
        delete spectrumContent;
    }
    spectrumContent = new uint8_t[lastResponse.contentLength];
    copy_response_content(spectrumContent);
    rawSpectrum.spectrumHeader = (hACQSpectrumHeader*)spectrumContent;

    spectrumStart = (uint16_t *)(spectrumContent + sizeof(hACQSpectrumHeader));
    spectrumEnd = spectrumStart + rawSpectrum.spectrumHeader->numPoints*rawSpectrum.spectrumHeader->numChannels;
    
    std::vector<uint16_t> rawSpectrumData(spectrumStart, spectrumEnd);
    
    rawSpectrum.rawSpectrumData.swap(rawSpectrumData);
    
    return rawSpectrum;

}


void Hyperion::set_peak_stream_divider(int streamingDivider)
{

    lastResponse = comm->execute_command("#SetPeakDataStreamingDivider", std::to_string(streamingDivider), 0);
}

void Hyperion::enable_peak_streaming(int streamingDivider, hComm* sComm)
{
    set_peak_stream_divider(streamingDivider);
    lastResponse = comm->execute_command("#EnablePeakDataStreaming", "", hREQUEST_OPT_NONE);
    
        if(sComm == nullptr)
    {
        peakStreamComm = new hCommTCPSocket(comm->get_address(), H_PEAK_STREAM_PORT);
    }
    else
    {
        peakStreamComm = sComm;
    }
    
    peakStreamComm->connect();
}

const hACQPeaks Hyperion::stream_peaks()
{
    double * peakData;
    lastResponse = peakStreamComm->read_response();
    if(peakContent != nullptr)
    {
        delete peakContent;
    }
    
    peakContent = new uint8_t[lastResponse.contentLength];
    
    copy_response_content(peakContent);
    peaksHeader = (hACQPeaksHeader*)peakContent;
    peakData = (double *)(peakContent + sizeof(hACQPeaksHeader));
    
    return  hACQPeaks(peaksHeader->peakCounts, peakData, *peaksHeader);

}

void Hyperion::disable_peak_streaming()
{
    lastResponse = comm->execute_command("#DisablePeakDataStreaming","",hREQUEST_OPT_NONE);
    
    peakStreamComm->close();
    delete peakStreamComm;

    
}

int Hyperion::get_peak_streaming_status(int &availableBufferPercentage)
{
    int status;
    lastResponse = comm->execute_command("#GetPeakDataStreamingStatus","", hREQUEST_OPT_NONE);
    
    status = *(int32_t*)lastResponse.content;
    if (status && peakStreamComm && peakStreamComm->is_connected())
    {
        status = 1;
    }
    else
    {
        status = 0;
    }
    lastResponse = comm->execute_command("#GetPeakDataStreamingAvailableBuffer","", hREQUEST_OPT_NONE);
    
    availableBufferPercentage = *(int32_t*)lastResponse.content;
    
    return status;
}

void Hyperion::set_spectrum_stream_divider(int streamingDivider)
{
    
    lastResponse = comm->execute_command("#SetFullSpectrumDataStreamingDivider", std::to_string(streamingDivider),hREQUEST_OPT_NONE);
}

void Hyperion::enable_spectrum_streaming(int streamingDivider, hComm* sComm)
{
    set_spectrum_stream_divider(streamingDivider);
    lastResponse = comm->execute_command("#EnableFullSpectrumDataStreaming", "", hREQUEST_OPT_NONE);
    
    if(sComm == nullptr)
    {
        spectrumStreamComm = new hCommTCPSocket(comm->get_address(), H_SPECTRUM_STREAM_PORT);
    }
    else
    {
        spectrumStreamComm = sComm;
    }
    spectrumStreamComm->connect();
}

const hACQSpectrum Hyperion::stream_spectrum()
{
    hACQSpectrum rawSpectrum;
    
    lastResponse = spectrumStreamComm->read_response();
    
    if(spectrumContent != nullptr)
    {
        delete spectrumContent;
    }
    spectrumContent = new uint8_t[lastResponse.contentLength];
    copy_response_content(spectrumContent);
    rawSpectrum.spectrumHeader = (hACQSpectrumHeader*)spectrumContent;
    
    uint16_t* spectrumStart = (uint16_t *)(spectrumContent + sizeof(hACQSpectrumHeader));
    uint16_t* spectrumEnd = spectrumStart + rawSpectrum.spectrumHeader->numPoints*rawSpectrum.spectrumHeader->numChannels;
    
    std::vector<uint16_t> rawSpectrumData(spectrumStart, spectrumEnd);
    
    rawSpectrum.rawSpectrumData.swap(rawSpectrumData);
    
    return rawSpectrum;

}

void Hyperion::disable_spectrum_streaming()
{
    lastResponse = comm->execute_command("#DisableFullSpectrumDataStreaming","",hREQUEST_OPT_NONE);
    
    spectrumStreamComm->close();
    delete spectrumStreamComm;

    
}

int Hyperion::get_spectrum_streaming_status(int &availableBufferPercentage)
{
    lastResponse = comm->execute_command("#GetFullSpectrumDataStreamingStatus","",hREQUEST_OPT_NONE);
    
    int status = *(int32_t*)lastResponse.content;
    if (status && spectrumStreamComm && spectrumStreamComm->is_connected())
    {
        status = 1;
    }
    else
    {
        status = 0;
    }
    lastResponse = comm->execute_command("#GetFullSpectrumDataStreamingAvailableBuffer","", hREQUEST_OPT_NONE);
    
    availableBufferPercentage = *(int32_t*)lastResponse.content;
    
    return status;
}

int Hyperion::get_laser_scan_speed()
{
    lastResponse = comm->execute_command("#GetLaserScanSpeed", "", hREQUEST_OPT_NONE);
    
    return  *(int32_t*)lastResponse.content;
}


std::vector<int>  Hyperion::get_available_laser_scan_speeds()
{
    lastResponse = comm->execute_command("#GetAvailableLaserScanSpeeds", "", hREQUEST_OPT_NONE);
    
    int numSpeeds = lastResponse.contentLength/(sizeof(int32_t));
    int32_t* speeds = (int32_t*)lastResponse.content;
    std::vector<int> speedsVec(speeds, speeds + numSpeeds);
    
    return speedsVec;
}


void Hyperion::set_laser_scan_speed(int scanSpeed)
{
    lastResponse = comm->execute_command("#SetLaserScanSpeed", std::to_string(scanSpeed), hREQUEST_OPT_NONE);
}



void Hyperion::get_power_calibration_info(std::vector<int> &offset, std::vector<int> &scale)
{
    lastResponse = comm->execute_command("#GetPowerCalibrationInfo", "", hREQUEST_OPT_NONE);
    
    int numData = lastResponse.contentLength/(sizeof(int32_t));
    int32_t* calData = (int32_t*)lastResponse.content;
    offset.clear();
    scale.clear();
    
    for(int index = 0; index < numData; index += 2)
    {
        offset.push_back(calData[index]);
        scale.push_back(calData[index + 1]);
    }
}

hNetworkSettings Hyperion::get_active_network_settings()
{
	char addr[INET_ADDRSTRLEN];
    
    hNetworkSettings networkSettings;
    
    lastResponse = comm->execute_command("#GetActiveNetworkSettings", "", hREQUEST_OPT_SUPPRESS_MSG);
    
    inet_ntop(AF_INET, ((in_addr*)&(lastResponse.content[0])), addr, INET_ADDRSTRLEN);
    networkSettings.ipAddress = addr;
    
    inet_ntop(AF_INET, ((in_addr*)&(lastResponse.content[4])), addr, INET_ADDRSTRLEN);
    networkSettings.mask = addr;
    
    inet_ntop(AF_INET, ((in_addr*)&(lastResponse.content[8])), addr, INET_ADDRSTRLEN);
    networkSettings.gateway = addr;
    
    return networkSettings;

    
}

std::string Hyperion::get_network_ip_mode()
{
    lastResponse = comm->execute_command("#GetNetworkIpMode", "", hREQUEST_OPT_SUPPRESS_MSG);
    
    return  std::string((char *)(lastResponse.content),lastResponse.contentLength);

}

hNetworkSettings Hyperion::get_static_network_settings()
{
    char addr[INET_ADDRSTRLEN];
    
    hNetworkSettings networkSettings;
    
    lastResponse = comm->execute_command("#GetStaticNetworkSettings", "", hREQUEST_OPT_SUPPRESS_MSG);
    
    inet_ntop(AF_INET, ((in_addr*)&(lastResponse.content[0])), addr, INET_ADDRSTRLEN);
    networkSettings.ipAddress = addr;
    
    inet_ntop(AF_INET, ((in_addr*)&(lastResponse.content[4])), addr, INET_ADDRSTRLEN);
    networkSettings.mask = addr;
    
    inet_ntop(AF_INET, ((in_addr*)&(lastResponse.content[8])), addr, INET_ADDRSTRLEN);
    networkSettings.gateway = addr;
    
    return networkSettings;
    
    
}

void Hyperion::set_network_ip_mode(hNetworkIPMode ipMode)
{
    std::string command;
    if(ipMode == DHCP)
    {
        command = "#EnableDynamicIpMode";
    }
    else if(ipMode == STATIC)
    {
        command = "#EnableStaticIpMode";
    }
    
    lastResponse = comm->execute_command(command, "", hREQUEST_OPT_NONE);
    close_comm();

}

void Hyperion::set_static_network_settings(std::string ipAddress, std::string netMask, std::string gateway)
{
    std::string currentIPMode = get_network_ip_mode();
    
    std::string argString;
    argString = ipAddress + " " + netMask + " " + gateway;
    lastResponse = comm->execute_command("#SetStaticNetworkSettings", argString, hREQUEST_OPT_NONE);
    //If the system is already in static mode, then this will invalidate the comm port, so close it
    if (currentIPMode == "STATIC")
    {
        close_comm();
    }
}


int Hyperion::get_last_error(std::string &errorMessage)
{
    return comm->get_last_error(errorMessage);
}

void Hyperion::close_comm()
{
    if(comm->is_connected())
        comm->close();
    if(peakStreamComm != nullptr && peakStreamComm->is_connected())
        peakStreamComm->close();
    if(spectrumStreamComm != nullptr && spectrumStreamComm->is_connected())
        spectrumStreamComm->close();
}

void Hyperion::connect_comm()
{
    if(! comm->is_connected())
        comm->connect();
}


