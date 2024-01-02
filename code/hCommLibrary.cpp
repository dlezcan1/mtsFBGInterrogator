//
//  hCommLibrary.cpp
//  hLibTest
//
//  Created by Dustin Carr on 2/6/15.
//  Copyright (c) 2015 Micron Optics, Inc. All rights reserved.
//


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
#include <cmath>
#include <fcntl.h>
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


#include "hyperion/hCommLibrary.h"


//------------------------------------------------------------------------------------
//
//                              hComm methods
//
//------------------------------------------------------------------------------------

const hResponse hComm::read_response()
{
    hReadHeader readHeader;
    int numRead;
    char* message;
    if (lastResponse.contentLength)
    {
        delete lastResponse.content;
        lastResponse.contentLength = 0;
    }
    numRead = read_data(sizeof(readHeader), (uint8_t*)&readHeader);
    if(numRead == -1)
    {
        throw HyperionException(this);
    }
    lastResponse.contentLength = readHeader.contentLength;
    lastResponse.messageLength = readHeader.messageLength;
    
    if(readHeader.messageLength)
    {
        message = new char[readHeader.messageLength];
        numRead = read_data(readHeader.messageLength, (uint8_t*)message);
        if(numRead == -1)
        {
            throw HyperionException(this);
        }
        
        lastResponse.message = std::string(message, numRead);
        delete message;
    }
    else
    {
        lastResponse.message = "";
    }
    
    if(readHeader.contentLength)
    {
        lastResponse.content = new uint8_t[lastResponse.contentLength];
        numRead = read_data(readHeader.contentLength, lastResponse.content);
    }
    if(numRead == -1)
    {
        throw HyperionException(this);
    }
    
    if (readHeader.status != H_SUCCESS)
    {
        throw HyperionException(readHeader.status, lastResponse.message);
    }
    return lastResponse;
    
}

const hResponse hComm::execute_command(std::string command, std::string argument, uint8_t requestOptions)
{
    write_command(command, argument, requestOptions);
    return read_response();
}

int hComm::write_command(std::string command, std::string argument, uint8_t requestOptions)
{
    ssize_t returnVal;
    hWriteHeader writeHeader;
    
    writeHeader.requestOption = requestOptions;
    writeHeader.commandSize = command.size();
    writeHeader.argSize = uint32_t(argument.size());
    
    char* commandString;
    //commandString = new char[writeHeader.commandSize];
    //strcpy_s(commandString, writeHeader.commandSize, command.c_str());
    returnVal = write_data((char *)&writeHeader, sizeof(writeHeader));	//write data header
    if (returnVal == -1)
    {
        throw HyperionException(this);
    }
    returnVal = write_data(command.c_str(), (int)writeHeader.commandSize);	//write end of command
    if (returnVal == -1)
    {
        throw HyperionException(this);
    }
    if (writeHeader.argSize)  //argument present
    {
        returnVal = write_data(argument.c_str(), (int)writeHeader.argSize); //write the argument
    }
    if (returnVal == -1)
    {
        throw HyperionException(this);
    }
    return 0;
    
}

void hComm::set_last_error(int errorNumber, std::string errorMessage)
{
    lastError = errorNumber;
    lastErrorMsg = errorMessage;
}


//------------------------------------------------------------------------------------
//
//                              hCommTCPSocket methods
//
//------------------------------------------------------------------------------------


hCommTCPSocket::hCommTCPSocket(std::string ipAddress, int port, int tout) :
ipAddress(ipAddress), port(port)
{
    lastResponse.contentLength = 0;
    lastResponse.messageLength = 0;
    lastError = 0;
    timeout = tout;
    connected = false;
    persistent = false;    
}

hCommTCPSocket::hCommTCPSocket(int sockfd, std::string command, std::string argument) : sockfd(sockfd)
{
    lastResponse.contentLength = 0;
    lastResponse.messageLength = 0;
    lastError = 0;
    connected = true;
    //Set persistent to true, since we want the sockfd to remain valid even if this hComm object goes away
    persistent = true;
    char addr[INET_ADDRSTRLEN];
    sockaddr addrInfo;
    socklen_t addrSize = sizeof(sockaddr);
    getpeername(sockfd,&addrInfo , &addrSize);
    inet_ntop(AF_INET, ((in_addr*)&(addrInfo.sa_data[2])), addr, INET_ADDRSTRLEN);
    ipAddress = addr;
    if(command.length())
    {
        execute_command(command, argument, 0); //Result stored in lastResponse
    }
    
}

hCommTCPSocket::~hCommTCPSocket()
{
    if(connected && !persistent)
    {
        close();
    }
    
    if (lastResponse.contentLength)
    {
        delete lastResponse.content;
    }
    
}

void hCommTCPSocket::set_last_error(int errorNumber, std::string errorMessage)
{
    if(errorNumber == -1)
    {
#ifdef _WIN32
        lastError = WSAGetLastError();
#else
        lastError = errno;
#endif
    }
    else
    {
        lastError = errorNumber;
    }
    
    lastErrorMsg = errorMessage;
}


int hCommTCPSocket::get_last_error(std::string &errorMsg)
{
    
    set_last_error();
    
#ifdef _WIN32
    char* charErrorMsg = NULL;
    if (lastError)
    {
        if(lastErrorMsg == "")
        {
            FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                           NULL, lastError,
                           MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                           (LPSTR)&charErrorMsg, 0, NULL);
            errorMsg = charErrorMsg;
            LocalFree(charErrorMsg);
        }
        else
        {
            errorMsg = lastErrorMsg;
        }
    }
    
    else
        errorMsg = "";
#else
    if(lastErrorMsg == "")
    {
        char buffer[256];
        strerror_r(lastError, buffer, 256);
        errorMsg = buffer;
    }
    else
        errorMsg = lastErrorMsg;
#endif
    return lastError;
    
    
}




int hCommTCPSocket::connect()
{
    if(!connected)
    {
        int len, fcntlArg;
        struct sockaddr_in address;
        int result, sockConnErr;
        timeval to;
        double toSec, fracPart, intPart;
        fd_set sockFdSet;
        toSec = double(timeout)/1000.0;
        fracPart = modf(toSec, &intPart);
        to.tv_sec = time_t(toSec);
        to.tv_usec = long(fracPart*1e6);
#ifdef _WIN32
        WSADATA wsData;
        WSAStartup(0x0202, &wsData);
#endif
        //create a socket endpoint
        sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sockfd == -1)
        {
            throw(HyperionException(this));
        }
        
        address.sin_family = AF_INET;
        //address.sin_addr.s_addr = inet_addr(ipAddress.c_str());
        inet_pton(address.sin_family, ipAddress.c_str(), &(address.sin_addr));
        address.sin_port = htons(port);
        len = sizeof(address);
        
        set_timeout(timeout);
        
        //In order to put a timeout on the connection, we set to nonblocking and
        //then use select() to wait on the connection with a specified timeout.
#ifdef _WIN32
        u_long ioctlArg = 1;
        ioctlsocket(sockfd, FIONBIO, &ioctlArg );
        
#else
        fcntlArg = fcntl(sockfd, F_GETFL, NULL);
        fcntlArg |= O_NONBLOCK;
        fcntl(sockfd, F_SETFL, fcntlArg);
#endif
        
        
        //connect the socket fd to the instrument
#ifdef _WIN32
        int inProgressErrorNumber = WSAEWOULDBLOCK;
#else
        int inProgressErrorNumber = EINPROGRESS;
#endif
        
        result = ::connect(sockfd, (struct sockaddr *)&address, len);
        if (result == -1)
        {
            set_last_error();
            if(lastError == inProgressErrorNumber)
            {
                set_last_error(0, "");
                FD_ZERO(&sockFdSet);
                FD_SET(sockfd, &sockFdSet);
                if (select(sockfd+1, NULL, &sockFdSet, NULL, &to) > 0)
                {
                    size_t errSize = sizeof(int);
#ifdef _WIN32
                    getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (char*)(&sockConnErr), (socklen_t*)&errSize);
#else
                    getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (void*)(&sockConnErr), (socklen_t*)&errSize);
#endif
                    if (sockConnErr)
                    {
                        throw HyperionException(sockConnErr);
                    }
                }
                else
                {
                    throw HyperionException(-22000, "Connection Timeout.  Check Address.");
                }
                
                
            }
            else
            {
                throw HyperionException(this);
            }
        }
        //Disable non-blocking mode
#ifdef _WIN32
        ioctlArg = 0;
        ioctlsocket(sockfd, FIONBIO, &ioctlArg);
        
#else
        fcntlArg = fcntl(sockfd, F_GETFL, NULL);
        fcntlArg &= (~O_NONBLOCK);
        fcntl(sockfd, F_SETFL, fcntlArg);
#endif
        connected = true;
    }
    return 0;
}

int hCommTCPSocket::close()
{
#ifdef _WIN32
    shutdown(sockfd, SD_BOTH);
    closesocket(sockfd);
    WSACleanup();
#else
    ::shutdown(sockfd, SHUT_RDWR);
    ::close(sockfd);
#endif
    connected = false;
    return 0;
}
int hCommTCPSocket::read_data(uint32_t numBytes, uint8_t* data)
{
    
    int numRead = 0;
    ssize_t returnVal;
    while (numRead != numBytes)
    {
        
        returnVal = recv(sockfd, (char *)&data[numRead], numBytes - numRead, 0);
        if (returnVal == -1)
        {
            return returnVal;
        }
        numRead += returnVal;
    }
    return numRead;
}

int hCommTCPSocket::set_timeout(int timeout)
{
    int returnVal = 0;
    
    double intPart, fracPart, toSec;
    toSec = double(timeout)/1000.0;
    fracPart = modf(toSec, &intPart);
#ifdef _WIN32

	int to = timeout;

#else
	timeval to;
	to.tv_sec = time_t(toSec);
    to.tv_usec = long(fracPart*1e6);
#endif
    
    returnVal = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&to, sizeof(to));
    
    if (returnVal == -1)
    {
        throw HyperionException(this);
    }
    
    returnVal = setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (const char *)&to, sizeof(to));
    
    if (returnVal == -1)
    {
        throw HyperionException(this);
    }
    
    return 0;
}

ssize_t hCommTCPSocket::write_data(const char * data, size_t dataCount)
{
    int returnVal;
    returnVal =  send(sockfd, data, dataCount, 0);
    return returnVal;
    
}

