//
//  hCommLibrary.h
//  hLibTest
//
//  Created by Dustin Carr on 2/6/15.
//  Copyright (c) 2015 Micron Optics, Inc. All rights reserved.
//

#ifndef __hLibTest__hCommLibrary__
#define __hLibTest__hCommLibrary__

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <exception>
#include <cstdint>
#include <time.h>

#ifdef _WIN32
typedef size_t ssize_t;
void sleep(double seconds);

#endif

#define		H_CMD_PORT          51971
#define     H_PEAK_STREAM_PORT       51972
#define     H_SPECTRUM_STREAM_PORT 51973
#define     H_DEFAULT_TIMEOUT   10000

#define     H_SUCCESS 0



/*!
 header that is written out before every command
 
 @field requestOption uint8_t containing flags that determine the output format returned from the instrument.
 
 @field reserved uint8_t reserved for future use.
 
 @field commandSize uint16_t the number of bytes in the command to be written.
 
 @field argSize uint32_t the number of bytes in the argument to be written.
 */

struct hWriteHeader
{
    uint8_t requestOption;
    uint8_t reserved;
    uint16_t commandSize;
    uint32_t argSize;
};


/*!
 header that is received with every response from the instrument
 
 @field status uint8_t The status of the command execution.
 
 @field requestOptionEcho uint8_t A copy of the request option that was originally sent to the instrument when the command was written.
 @field messageLength uint16_t The length of the returned message, in bytes.
 
 @field contentLength uint32_t The length of the returned content, in bytes.
 */

struct hReadHeader
{
    uint8_t status;
    uint8_t requestOptionEcho;
    uint16_t messageLength;
    uint32_t contentLength;
};



/*!
 Encapsulates the returned response from an executed command.
 
 @field messageLength uint16_t The length of the returned message, in bytes.
 
 @field message string containing the human-readable message that is returned from the instrument.
 
 @field contentLength uint32_t The length of the returned content, in bytes.
 
 @field content Pointer to a buffer containing the returned data.
 */

struct hResponse
{
    uint16_t messageLength;
    std::string message;
    uint32_t contentLength;
    uint8_t* content;
};



/*!
 
 class that defines the interface to a Hyperion instrument
 
 */

class hComm
{
protected:
    hResponse lastResponse;
    bool connected;
    int timeout;
    int lastError;
    std::string lastErrorMsg;
    
    /*!
     Read raw data back from a communication channel (virtual).
     
     @param numBytes the number of Bytes to be returned.
     
     @param data the data returned as an array of bytes.
     
     @return returns the number of bytes read, or -1 if there is an error
     
     */
    virtual int read_data(uint32_t numBytes, uint8_t data[]) = 0;
    
    /*!
     Write raw data to a communication channel (virtual)
     
     @param data Data buffer to write.
     
     @param dataCount number of dataBytes to write out
     
     @return Returns number of bytes written if successful, -1 otherwise
     */
    
    virtual ssize_t write_data(const char * data, size_t dataCount) = 0;
    
    
    virtual void set_last_error(int errorNumber = -1, std::string errorMessage = "");
    
public:
    
    /*!
     Destructor for hComm class (virtual)
     */
    
    virtual ~hComm() {}
    
    /*!
     close the communication channel to the instrument (virtual)
     */
    
    virtual int close() = 0;
    
    /*!
     open a communication channel and establish a connection (virtual)
     */
    
    virtual int connect() = 0;
    /*!
     Execute a hyperion command by sending a command and reading a response.
     
     @param command The hyperion command to be sent.
     
     @param argument The command argument.
     
     @param requestOptions options that determine the output returned by the hyperion Instrument
     
     @return 0 if successful, 1 if there is an error returned by the instrument, or -1 if there is a system error.
     If there is an error returned by the instrument, then the error message will be contained in response.message.
     If there is a system error, use get_last_error() to get the error message.
     
     */
    const hResponse execute_command(std::string command, std::string argument, uint8_t requestOptions);
    
    
    
    /*!
     Read a structured response from a Hyperion instrument.  Throws HyperionException on error.
     
     @return 0 if successful, 1 if there is an error returned by the instrument, or -1 if there is a system error.
     If there is an error returned by the instrument, then the error message will be contained in response.message.
     If there is a system error, use get_last_error() to get the error message.
     
     
     */
    
    const hResponse read_response();
    
    virtual int get_last_error(std::string &errorMsg) = 0;
    
    
    /*!
     set the timeout value in milliseconds for the specified channel. (virtual)
     */
    virtual int set_timeout(int timeout) = 0;
    
    /*!
     Write a hyperion command to the instrument.  Throws HyperionException on error.
     
     @param command The hyperion command to be sent.
     
     @param argument The command argument.
     
     @param requestOptions options the determine the output returned by the hyperion Instrument
     
     */
    int write_command(std::string command, std::string argument, uint8_t requestOptions);
    
    virtual std::string get_address() = 0;
    
    const hResponse get_last_response()
    {
        return lastResponse;
    }
    
    
    bool is_connected()
    {
        return connected;
    }
};

/*!
 
 a class that defines a TCP interface to a Hyperion Instrument
 
 */

class hCommTCPSocket:public hComm
{
private:
    uint8_t* readBuffer;
    std::string ipAddress;
    int port;
    int sockfd;
    bool persistent;
    
    /*!
     Read raw data back from a communication channel.
     
     @param numBytes the number of Bytes to be returned.
     
     @param data the data returned as an array of bytes.
     
     @return returns the number of bytes read, or -1 if there is an error
     
     */
    
    int read_data(uint32_t numBytes, uint8_t data[]);
    
    /*!
     Write raw data to a communication channel
     
     @param data Data buffer to write.
     
     @param dataCount number of dataBytes to write out
     
     @return Returns number of bytes written if successful, -1 otherwise
     */
    
    ssize_t write_data(const char * data, size_t dataCount);
    
    virtual void set_last_error(int errorNumber = -1, std::string errorMessage = "");;
    
public:
    /*!
     Constructor for hCommTCPSocket
     
     @param ipAddress The TCP/IPv4 address of the instrument in #.#.#.# format.
     
     @param port The port number to connect to.
     
     @param timeout The defaul timeout for all TCP communications.
     */
    
    hCommTCPSocket(std::string ipAddress, int port = H_CMD_PORT, int timeout = H_DEFAULT_TIMEOUT);
    
    /*!
     Constructor for hCommTCPSocket.  This version takes an already initialized and valid socket number and binds the comm object to that.  It can also optionally execute a command.
     
     @param sockfd A valid socket file descriptor.
     
     @param command A command to be immediately executed.
     
     @param argument Argument to the command to be executed.
     */
    
    hCommTCPSocket(int sockfd, std::string command = "", std::string argument = "");
    
    /*!
     Destructor for hCommTCPSocket
     */
    ~hCommTCPSocket();
    
    /*!
     close the communication channel to the instrument.
     */
    
    
    int close();
    
    int get_last_error(std::string &errorMsg);
    
    /*!
     open a communication channel and establish a connection. Throws HyperionException on error.
     */
    
    int connect();
    
    
    /*!
     set the timeout value in milliseconds for the specified channel.  Throws HyperionException on error.
     
     @param timeout The timeout value in milliseconds
     */
    
    int set_timeout(int timeout);
    
    
    std::string get_address()
    {
        return ipAddress;
    }
    void set_persistent(bool persistence)
    {
        persistent = persistence;
    }
    bool get_persistent()
    {
        return persistent;
    }
    int get_sockfd()
    {
        return sockfd;
    }
    
};

/*!
 Exception class used by the hCommLibrary.
 
 */

class HyperionException: public std::exception
{
private:
    int errorNumber;
    std::string errorMessage;
public:
    /*!
     Construct for HyperionException.  In this constructor, the errorNumber and errorMessage are supplied manually
     
     @param errorNumber The errorNumber to assign to the exception.
     
     @param errorMessage A String containing a description of the error
     */
    HyperionException(int errorNumber, std::string errorMessage = "") : errorNumber(errorNumber), errorMessage(errorMessage)
    {}
    
    /*!
     Constructor for HyperionException.  This constructor takes a pointer to an hComm object, from which the detailed error information is retrieved.
     
     @param errorSource A pointer to the hComm object that is throwing the exception.
     */
    HyperionException(hComm* errorSource)
    {
        errorNumber = errorSource->get_last_error(errorMessage);
    }
    /*!
     Overloaded function that returns a string describing the exception
     
     @return Returns a pointer to the descriptive string.
     */
    virtual const char* what() const throw()
    {
        std::string errorDescription = std::to_string(errorNumber) + ":  " + errorMessage;
        return errorDescription.c_str();
    }
    ~HyperionException() throw() {}
};


#endif /* defined(__hLibTest__hCommLibrary__) */
