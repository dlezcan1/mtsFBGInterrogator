//
//  hLibrary.h
//  hLibrary
//
//  Created by Dustin Carr on 8/6/14.
//  Copyright (c) 2014 Micron Optics, Inc. All rights reserved.
//

#ifndef __hLibrary__hLibrary__
#define __hLibrary__hLibrary__

#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <time.h>

#include "hCommLibrary.h"

#define H_LIB_VERSION "0.9.5.0"

#define SUCCESS 0


/* ============================================================================
 Misc. Hyperion constants
 ============================================================================ */

#define H_MAX_NUM_CHANNELS 16

#define USER_DATA_BUFFER_LENGTH 65534


enum hNetworkIPMode {STATIC, DHCP};


/* ============================================================================
 Misc. Hyperion structures
 ============================================================================ */

//---------- Hyperion Write Command Request Options ----------
#define hREQUEST_OPT_NONE				0
#define hREQUEST_OPT_SUPPRESS_MSG		1
#define hREQUEST_OPT_SUPPRESS_CONTENT	2
#define hREQUEST_OPT_COMPRESS			4




/*!
 Structure containing network settings
 
 @field ipAddress An IP address in #.#.#.# format.
 
 @field mask Network mask in #.#.#.# format.
 
 @field gateway Network gateway in #.#.#.# format.
 */

struct hNetworkSettings
{
    std::string ipAddress;
    std::string mask;
    std::string gateway;
};

/*!
 Header data returned with each call to Hyperion.get_peaks or Hyperion.stream_peaks

 @field length uint16_t The lenght of the header.
 
 @field version The version of the header implementation.
 
 @field reserved Reserved for future use.
 
 @field serialNumber uint64_t Sequential number assigned to each sample of peak data.
 
 @field timeStampInt Timestamp for data, expressed as number of seconds since 00:00:00, Jan. 1, 1970
 
 @field timeStampFrac Number of nanoseconds elapsed after timeStampInt
 
 @field peakCounts Number of peaks detected for each channel
 */

struct hACQPeaksHeader
{
    uint16_t length;
    uint16_t version;
    uint32_t reserved;
    uint64_t serialNumber;
    uint32_t timeStampInt;
    uint32_t timeStampFrac;
    uint16_t peakCounts[H_MAX_NUM_CHANNELS];
};


/*!
 Peak data returned from a call to Hyperion.get_peaks() or Hyperion.stream_peaks()

 */

class hACQPeaks
{
private:
    
    uint16_t startInds[H_MAX_NUM_CHANNELS];
    uint16_t endInds[H_MAX_NUM_CHANNELS];
    
    double* peaks;
    uint16_t numPeaks;
    
    
public:
    
    double timeStamp;
    uint64_t serialNumber;
    
    /*!
     Constructor
     
     @param peakCounts Array containing number of peaks detected on each channel
     
     @param peakData Array containing all of the detected peak wavelengths
     */
    
    
    hACQPeaks(uint16_t peakCounts[H_MAX_NUM_CHANNELS], double* peakData,
              const hACQPeaksHeader peaksHeader)
    {
        peaks = peakData;
        int currentInd = 0;
        for(int i = 0; i < H_MAX_NUM_CHANNELS; i++)
        {
            startInds[i] = currentInd;
            currentInd += peakCounts[i];
            endInds[i] = currentInd;
        }
        numPeaks = currentInd;
        
        timeStamp = double(peaksHeader.timeStampInt) +
                    double(peaksHeader.timeStampFrac)*1e-9;
        serialNumber = peaksHeader.serialNumber;
    }
    
    /*!
     Returns all of the peaks on the given channel
     
     @param channel The channel number for which the peaks will be returned.
     
     @return a vector containing all of the peaks on the channel.
     */
    
    std::vector<double> get_channel(uint16_t channel)
    {
        std::vector<double> channelPeaks;
        channel = channel - 1;
        for(int i = startInds[channel];i < endInds[channel]; i++)
        {
            channelPeaks.push_back(peaks[i]);
        }
        
        return channelPeaks;
        
    }
    
    /*!
     Returns all of the peaks on all channels in a single vector.
     
     @return a vector containing all of the peaks on all channels
     */
    std::vector<double> get_all()
    {
        std::vector<double> allPeaks;
        for(int i = 0; i < numPeaks; i++)
        {
            allPeaks.push_back(peaks[i]);
        }
        return allPeaks;
    }
    /*!
     Returns the total number of peaks across all channels.
     
     @return The total number of peaks on all channels.
     */
    
    int get_num_peaks()
    {
        return numPeaks;
    }
    
};

/*!
 Header returned with each call to Hyperion.get_spectrum()
 
 @field length The length of this header in bytes.
 
 @field version The version number of this header format.
 
 @field reserved Reserve for future use
 
 @field serialNumber uint64_t Sequential number assigned to each sample of peak data.
 
 @field timeStampInt Timestamp for data, expressed as number of seconds since 00:00:00, Jan. 1, 1970
 
 @field timeStampFrac Number of nanoseconds elapsed after timeStampInt
 
 @field startWavelength The wavelength of the first datapoint in the acquired spectrum.
 
 @field wavelengthIncrement The wavelength increment for each datapoint in the spectrum.
 
 @field numPoints The number of points in the spectrum.
 
 @field numChannels The number of channels returned.
 
 @field reserved2 Reserved for future use.
 */

struct hACQSpectrumHeader
{
    uint16_t length;
    uint16_t version;
    uint32_t reserved;
    uint64_t serialNumber;
    uint32_t timeStampInt;
    uint32_t timeStampFrac;
    double startWavelength;
    double wavelengthIncrement;
    uint32_t numPoints;
    uint16_t numChannels;
    uint16_t reserved2;
};

/*!
 Spectrum data returned with each call to Hyperion.get_raw_spectrum()
 
 @field spectrumHeader A pointer to the hACQSpectrumHeader structure for this spectrum
 
 @field rawSpectrumData A vector containing the raw spectrum data.
 
 @field calibratedSpectrumData A vector containing the calibrated spectrum data.  This may be empty if the spectrum was returned with get_raw_spectrum()
 */

struct hACQSpectrum
{
    hACQSpectrumHeader* spectrumHeader;
    std::vector<uint16_t> rawSpectrumData;
    std::vector<double> calibratedSpectrumData;
    
};



/*!
 Class that encapsulates the behavior of the hyperion instrument.
 */

class Hyperion
{
private:
    hComm* comm;
    hComm* peakStreamComm;
    hComm* spectrumStreamComm;
    hACQPeaksHeader* peaksHeader;
    
    hACQSpectrumHeader* spectrumHeader;
    hACQSpectrum spectrum;
    int spectrumChannel;
    hResponse lastResponse;
    uint8_t* peakContent;
    uint8_t* spectrumContent;
    
    void init();
    
    void copy_response_content(uint8_t* copyBuffer);
    void get_user_data(int slot, uint8_t* userDataBuffer);
    std::vector<int> calibrationOffset;
    std::vector<int> calibrationScale;
    std::vector<double> calibrationInvScale;
    
    double startWvl, deltaWvl;
    int numWvl;
    
    int numChannels;
    
public:
    /*!
     @methodgroup Constructors
     */
    
    /*!
     Constructor that takes a pointer to an hComm object (or descendant)
     */
    
    Hyperion(hComm* comm);
    
    /*!
     Constructor that takes TCP parameters.
     
     @param ipAddress The TCP/IPv4 address of the instrument in #.#.#.# format.
     
     @param port The TCP port to connect to.
     
     @param timeout The default timeout for all TCP communication.
     */
    
    Hyperion(std::string ipAddress, int port = H_CMD_PORT, int timeout = H_DEFAULT_TIMEOUT);
    
    /*!
     Destructor for Hyperion object
     */
    
    ~Hyperion();
    
    /*!
     @methodgroup System API
     
     */
    
    /*!
     Get the serial number of the Hyperion instrument
     
     @return Returns a string containing the serial number.
     */
    std::string get_serial_number();
    
    /*!
     Get the version of this API library
     
     @return Returns String containing the library version.
     
     */
    
    std::string get_library_version();
    
    /*!
     Returns the firmware, fpga, and libray versions
     
     @param fwVersion Contains the value of the firmware version.
     
     @param fPGAVersion Contains the value of the FPGA version.
     
     @param hLibraryVersion Contains the value of this API library version

     */
    
    void get_version(std::string &fwVersion, std::string &fPGAVersion, std::string &hLibraryVersion);
    
    /*!
     Set the instrument name.
     
     @param instrumentName A string containing the instrument name.
     
     */
    
    void set_instrument_name(std::string instrumentName);
    
    /*!
     Gets the instrument name
     
     @return Returns a string that contains the instrument name..
     
     */
    
    std::string get_instrument_name();
    
    /*!
     @methodgroup Acquisition API
     
     */
    
    /*!
     Acquires a set of peaks from the Hyperion instrument
     
     @return Returns a hACQPeaks structure that contains the peak data.
     
     */
    
    const hACQPeaks get_peaks();
    /*!
     Get the parameters that define the wavelength scan range and number of points.
     
     @param startWavelength The first wavelength in each scan, in units of nm.
     
     @param deltaWavelength The step size between neighboring wavelengths in the scan.
     
     @param numWavelengths The number of wavelengths in the scan.
     
     */
    void get_scan_parameters(double &startWavelength, double &deltaWavelength, int &numWavelengths);
    
    /*!
     Acquires a full spectrum data from the Hyperion instrument
     
     @param numChannelsToReturn number of channels specified in the channelsToReturn array.
     
     @param channelsToReturn array of channels
     
     @return Returns an hACQSpectrum structure containing the spectrum data.
     */
    
    const hACQSpectrum get_spectrum(int numChannelsToReturn = 0, int* channelsToReturn = nullptr);
    
    /*!
     Acquires a raw spectrum for the specified channel.  If channel == -1, then this will get
     data for all channels.
     
     @param channel The channel number to be acquired.  If this is -1, all channels will be returned.
     
     @return Resturns an hACQSpectrum structure containing the spectrum data.  Only the rawSpectrumData vector will contain data.
     */
    
    const hACQSpectrum get_raw_spectrum(int channel);
    
    
    /*!
     Sets the value of the peak streaming divider, which determines the rate at which streaming peak data is returned. if peakStreamingDivider = 5, then every 5th sample is streamed out on the peak streaming port.
     
     @param peakStreamingDivider Value to be applied.

     */
    
    void set_peak_stream_divider(int peakStreamingDivider);
    
    
    /*!
     
     Enable streaming of peak data from the instrument.
     
     @param streamingDivider Output rate divider.  Data will be output at the scan rate of the instrument divided by this.
     
     @param sComm hComm object to be used for streaming communication.  If null, then the instrument IP address and default streaming port will be used.
     
     */
    
    void enable_peak_streaming(int streamingDivider, hComm* sComm = nullptr);
    
    /*!
     Acquires a set of streamed peaks from the Hyperion instrument.
     
     @return Returns an hACQPeaks structure.
     
     */
    
    const hACQPeaks stream_peaks();
    /*!
     Disables streaming of peak data
     
     */
    
    void disable_peak_streaming();
    
    /*!
     Gets the current status and available buffer for peak streaming
     
     @param availableBufferPercentage The current percent of space available in the output buffer.  Use this to 
     verify that data is being consumed fast enough to avoid buffer overflows.
     
     @return Returns 1 if the peak streaming is enabled on the instrument AND the peak streaming comm
     port is open on the client, and 0 otherwise.
     */
    
    int get_peak_streaming_status(int &availableBufferPercentage);
    
    
    /*!
     Sets the value of the spectrum streaming divider, which determines the rate at which streaming spectrum data is returned. if peakStreamingDivider = 5, then every 5th spectrum is streamed out on the spectrum streaming port.
     
     @param spectrumStreamingDivider Value to be applied.
     */
    
    
    void set_spectrum_stream_divider(int spectrumStreamingDivider);
    
    
    /*!
     
     Enable streaming of spectrum data from the instrument.
     
     @param streamingDivider Output rate divider.  Data will be output at the scan rate of the instrument divided by this.
     
     @param sComm hComm object to be used for streaming communication.  If null, then the instrument IP address and default streaming port will be used.
     
     */
    
    
    
    void enable_spectrum_streaming(int streamingDivider, hComm* sComm = nullptr);
    
    /*!
     Acquires streamed spectrum data from the Hyperion instrument, and stores it in the spectrum instance variable.
     
     @return Resturns an hACQSpectrum structure containing the spectrum data.  Only the rawSpectrumData vector will contain data.
     */
    
    const hACQSpectrum stream_spectrum();
    
    /*!
     Disables streaming of spectrum data
     
     */
    
    void disable_spectrum_streaming();
    
    /*!
     Gets the current status and available buffer for spectrum streaming
     
     @param availableBufferPercentage The current percent of space available in the output buffer.  Use this to
     verify that data is being consumed fast enough to avoid buffer overflows.
     
     @return Returns 1 if the spectrum streaming is enabled on the instrument AND the spectrum streaming comm
     port is open on the client, and 0 otherwise.
     */
    
    int get_spectrum_streaming_status(int &availableBufferPercentage);
    
    /*!
     Gets the current laser scanning speed.
     
     @return Returns the current laser scanning rate in Hz.
     
     */
    int get_laser_scan_speed();
    
    /*!
     
     Gets the valid laser scan speeds that can be set on this instrument.
     
     @return returns A vector<int> containing all of the speeds that are supported on the instrument.
     
     */
    
    std::vector<int> get_available_laser_scan_speeds();
    
    /*!
     Sets the laser scan speed.
     
     @param scanSpeed The laser scan speed, in Hz.
     */
    void set_laser_scan_speed(int scanSpeed);
    
    /*!
     Gets the calibration offset and scale used for converting spectra to dBm power units.  Converting spectral data
     is done by taking spectrumData/scale + offset.
     
     @param offset A vector<int> containing the calibration offsets for each channel
     
     @param scale A vector<int> containing the calibration scales for each channel.
     
     */

    
    void get_power_calibration_info(std::vector<int> &offset, std::vector<int> &scale);
    
    
    /*!
     @methodgroup Net API
    
     */
    
    /*!
     Gets the current active network settings for the instrument
     
     @return Returns a hNetworkSettings structure that contains the returned network settings.
     */
    
    hNetworkSettings get_active_network_settings();
    
    /*!
     Gets the current network mode, static or dynamic (DHCP)
     
     @return Returns a string indicating the current network IP mode (STATIC or DHCP)
     
     */
    
    std::string get_network_ip_mode();
    
    /*!
     Gets the current static network settings.
     
     @return Returns a hNetworkSettings structure that contains the returned network settings.
     
     */
    
    hNetworkSettings get_static_network_settings();
    
    /*!
     sets the network IP mode (static or DHCP).  Note that if this is different than the current settings, then the hComm object will no longer be able to establish a connection and should be closed and re-initiated.
     
     
     @param ipMode The network IPMode.  Can be STATIC or DHCP.
     
     */
    
    void set_network_ip_mode(hNetworkIPMode ipMode);
    
    /*!
     sets the static network settings.  Note that if the current IP mode is static, then changing these parameters will result in not being able to communicate with the instrument and the hComm object should be closed and re-initiated.
     
     @param ipAddress the TCP/IPv4 address of the instrument in #.#.#.# format.
     
     @param netMask The network mask of the instrument in #.#.#.# format.
     
     @param gateway The network gateway for the instrument in #.#.#.# format
     
     */
    
    void set_static_network_settings(std::string ipAddress, std::string netMask, std::string gateway );
    
    /*!
     @methodgroup Utility Functions
     
     */
    
    /*!
     Returns a pointer to the active hComm object.
     */
    
    hComm* get_comm()
    {
        return comm;
    }
    /*!
     Returns the error message associated with the most recent instrument or system error.
     
     @param errorMessage Upon successful completion, this contains the error message.
     
     */
    
    int get_last_error(std::string &errorMessage);
    
    /*!
     Closes the comm and streamComm sockets if currently open.
     */
    
    void close_comm();
    
    
    /*!
     Re-connects the comm object if not already connected
     */
    
    void connect_comm();

    
};


#endif /* defined(__hLibrary__hLibrary__) */
