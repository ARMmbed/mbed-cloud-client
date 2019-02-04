/*******************************************************************************
 * Copyright 2016, 2017 ARM Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *******************************************************************************/

#define _GNU_SOURCE // This is for ppoll found in poll.h
#include "pal.h"
#include "pal_plat_network.h"
#include "pal_rtos.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
//#include <net/if.h>
#include <netdb.h>
//#include <ifaddrs.h>
#include <errno.h>
#if PAL_NET_ASYNCHRONOUS_SOCKET_API
#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dosextens.h>
#include <dos/dostags.h>
 
#include <proto/exec.h>
#include <proto/dos.h>

#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <assert.h>
#endif

#define TRACE_GROUP "PAL"

#ifdef PAL_NET_TCP_AND_TLS_SUPPORT
//#include <netinet/tcp.h>
#endif

PAL_PRIVATE void* s_pal_networkInterfacesSupported[PAL_MAX_SUPORTED_NET_INTERFACES] = { 0 };
PAL_PRIVATE uint32_t s_pal_numberOFInterfaces = 0;

PAL_PRIVATE palStatus_t translateErrorToPALError(int errnoValue)
{
    palStatus_t status;
    switch (errnoValue)
    {
    #if 0
    case EAI_MEMORY:
        status = PAL_ERR_NO_MEMORY;
        break;
    case EWOULDBLOCK:
        status = PAL_ERR_SOCKET_WOULD_BLOCK;
        break;
    case ENOTSOCK:
        status = PAL_ERR_SOCKET_INVALID_VALUE;
        break;
    case EPERM:
    case EACCES:
        status = PAL_ERR_SOCKET_OPERATION_NOT_PERMITTED;
        break;
    case ETIMEDOUT:
        status = PAL_ERR_TIMEOUT_EXPIRED;
        break;
    case EISCONN:
        status = PAL_ERR_SOCKET_ALREADY_CONNECTED;
        break;
    case EINPROGRESS:
        status = PAL_ERR_SOCKET_IN_PROGRES;
        break;
    case EALREADY:
        status = PAL_ERR_SOCKET_ALREADY_CONNECTED;
        break;
    case EINVAL:
        status = PAL_ERR_SOCKET_INVALID_VALUE;
        break;
    case EADDRINUSE:
        status = PAL_ERR_SOCKET_ADDRESS_IN_USE;
        break;
    case ECONNABORTED:
        status = PAL_ERR_SOCKET_CONNECTION_ABORTED;
        break;
    case ECONNRESET:
    case ECONNREFUSED:
        status = PAL_ERR_SOCKET_CONNECTION_RESET;
        break;
    case ENOBUFS:
    case ENOMEM:
        status = PAL_ERR_SOCKET_NO_BUFFERS;
        break;
    case EINTR:
        status = PAL_ERR_SOCKET_INTERRUPTED;
        break;
    #endif
    default:
        PAL_LOG_ERR("translateErrorToPALError() cannot translate %d", errnoValue);
        status = PAL_ERR_SOCKET_GENERIC;
        break;
    }
    return status;
}

#if PAL_NET_ASYNCHRONOUS_SOCKET_API
struct pollfd {
    int fd;
    short events;
    short revents;
};

static palMutexID_t s_mutexSocketEventFilter = 0;

// These must be updated only when protected by s_mutexSocketCallbacks
static struct pollfd s_fds[PAL_NET_TEST_MAX_ASYNC_SOCKETS] = {{0,0,0}};
static uint32_t s_callbackFilter[PAL_NET_TEST_MAX_ASYNC_SOCKETS] = {0};

PAL_PRIVATE void clearSocketFilter( int socketFD)
{
    palStatus_t result = PAL_SUCCESS;
    int i = 0;
    result = pal_osMutexWait(s_mutexSocketEventFilter, PAL_RTOS_WAIT_FOREVER);
    if (PAL_SUCCESS != result)
    {
        PAL_LOG_ERR("error waiting for mutex"); // we want to zero the flag even if this fails beacuse it is better to get an extra event than miss one.
    }
    for (i = 0; i < PAL_NET_TEST_MAX_ASYNC_SOCKETS; i++)
    {

        if (s_fds[i].fd == socketFD)
        {
            s_callbackFilter[i] = 0;
            break;
        }
    }
    result = pal_osMutexRelease(s_mutexSocketEventFilter);
    if (PAL_SUCCESS != result)
    {
        PAL_LOG_ERR("error releasing mutex");
    }
}

#endif

palStatus_t pal_plat_socketsInit(void* context)
{    
    palStatus_t result = PAL_SUCCESS;

    return result;
}

palStatus_t pal_plat_registerNetworkInterface(void* context, uint32_t* interfaceIndex)
{
    palStatus_t result = PAL_SUCCESS;
    uint32_t index = 0;
    bool found = false;

    for (index = 0; index < s_pal_numberOFInterfaces; index++) // if specific context already registered return exisitng index instead of registering again.
    {
        if (s_pal_networkInterfacesSupported[index] == context)
        {
            found = true;
            *interfaceIndex = index;
            break;
        }
    }

    if (false == found)
    {
        if (s_pal_numberOFInterfaces < PAL_MAX_SUPORTED_NET_INTERFACES)
        {
            s_pal_networkInterfacesSupported[s_pal_numberOFInterfaces] = context;
            *interfaceIndex = s_pal_numberOFInterfaces;
            ++s_pal_numberOFInterfaces;            
        }
        else
        {
            result = PAL_ERR_SOCKET_MAX_NUMBER_OF_INTERFACES_REACHED;
        }
    }

    return result;
}

palStatus_t pal_plat_unregisterNetworkInterface(uint32_t interfaceIndex)
{
    s_pal_networkInterfacesSupported[interfaceIndex] = NULL;
    --s_pal_numberOFInterfaces;
    return PAL_SUCCESS;
}

palStatus_t pal_plat_socketsTerminate(void* context)
{    
    palStatus_t result = PAL_SUCCESS;    

    return result;
}

palStatus_t pal_plat_socket(palSocketDomain_t domain, palSocketType_t type, bool nonBlockingSocket, uint32_t interfaceNum, palSocket_t* sockt)
{
    int result = PAL_SUCCESS;
    intptr_t sockfd;    

    PAL_VALIDATE_ARGUMENTS(interfaceNum >= s_pal_numberOFInterfaces && PAL_NET_DEFAULT_INTERFACE != interfaceNum);

    // These are the same in Linux
    if(type == PAL_SOCK_STREAM_SERVER)
    {
        type = PAL_SOCK_STREAM;
    }
    
    PAL_ASSERT_STATIC(AF_INET == PAL_AF_INET);
    //PAL_ASSERT_STATIC(AF_INET6 == PAL_AF_INET6);
    PAL_ASSERT_STATIC(AF_UNSPEC == PAL_AF_UNSPEC);
    PAL_ASSERT_STATIC(SOCK_DGRAM == (unsigned int)PAL_SOCK_DGRAM);
    PAL_ASSERT_STATIC(SOCK_STREAM == (unsigned int)PAL_SOCK_STREAM);
    
    sockfd = socket(domain, type, 0);
    // Note - though it is not an error, if we get sockfd == 0 then we probably (accidentally closed fd 0 somewhere else)
    if (sockfd == -1)
    {
        result = translateErrorToPALError(errno);
    }
    else
    {
        if (nonBlockingSocket)
        {
            fcntl( sockfd, F_SETFL, fcntl(sockfd, F_GETFL ) | O_NONBLOCK );
        }
        *sockt = (palSocket_t)sockfd;
    }
    return result;
}


// Assume input timeout value is in milliseconds.
palStatus_t pal_plat_setSocketOptions(palSocket_t socket, int optionName, const void* optionValue, palSocketLength_t optionLength)
{    
    return PAL_ERR_SOCKET_OPTION_NOT_SUPPORTED;
}

palStatus_t pal_plat_isNonBlocking(palSocket_t socket, bool* isNonBlocking)
{
    int flags = fcntl((intptr_t)socket, F_GETFL);

    if (0 != (flags & O_NONBLOCK))
    {
        *isNonBlocking = true;
    }
    else
    {
        *isNonBlocking = false;
    }
    return PAL_SUCCESS;
}

PAL_PRIVATE palStatus_t pal_plat_SockAddrToSocketAddress(const palSocketAddress_t* palAddr, struct sockaddr* output)
{
    palStatus_t result = PAL_SUCCESS;
    uint16_t port = 0;
    bool found = false;

    result = pal_getSockAddrPort(palAddr, &port);
    if (result != PAL_SUCCESS)
    {
        return result;
    }

#if PAL_SUPPORT_IP_V4
    if (PAL_AF_INET == palAddr->addressType)
    {
        palIpV4Addr_t ipV4Addr = { 0 };
        struct sockaddr_in* ip4addr = (struct sockaddr_in*)output;
        ip4addr->sin_family = AF_INET;
        ip4addr->sin_port = PAL_HTONS(port);
        result = pal_getSockAddrIPV4Addr(palAddr, ipV4Addr);
        if (result == PAL_SUCCESS)
        {
            memcpy(&ip4addr->sin_addr, ipV4Addr, sizeof(ip4addr->sin_addr));
        }
        found = true;
    }

#endif // PAL_SUPPORT_IP_V4
#if PAL_SUPPORT_IP_V6
    if (PAL_AF_INET6 == palAddr->addressType)
    {
        palIpV6Addr_t ipV6Addr = {0};
        struct sockaddr_in6* ip6addr = (struct sockaddr_in6*)output;
        ip6addr->sin6_family = AF_INET6;
        ip6addr->sin6_scope_id = 0; // we assume there will not be several interfaces with the same IP.
        ip6addr->sin6_flowinfo = 0;
        ip6addr->sin6_port = PAL_HTONS(port);
        result = pal_getSockAddrIPV6Addr(palAddr, ipV6Addr);
        if (result == PAL_SUCCESS)
        {
            memcpy(&ip6addr->sin6_addr, ipV6Addr, sizeof(ip6addr->sin6_addr));
        }
        found = true;
    }
#endif

    if (false == found)
    {
        return PAL_ERR_SOCKET_INVALID_ADDRESS;
    }

    return result;
}

PAL_PRIVATE palStatus_t pal_plat_socketAddressToPalSockAddr(struct sockaddr* input, palSocketAddress_t* out, palSocketLength_t* length)
{
    palStatus_t result = PAL_SUCCESS;
    bool found = false;

#if PAL_SUPPORT_IP_V4
    if (input->sa_family == AF_INET)
    {
        palIpV4Addr_t ipV4Addr;
        struct sockaddr_in* ip4addr = (struct sockaddr_in*)input;

        memcpy(ipV4Addr, &ip4addr->sin_addr, PAL_IPV4_ADDRESS_SIZE);
        result = pal_setSockAddrIPV4Addr(out, ipV4Addr);
        if (result == PAL_SUCCESS)
        {
            result = pal_setSockAddrPort(out, PAL_NTOHS(ip4addr->sin_port));
        }
        *length = sizeof(struct sockaddr_in);
        found = true;
    }
#endif //PAL_SUPPORT_IP_V4
#if PAL_SUPPORT_IP_V6
    if (input->sa_family == AF_INET6)
    {
        palIpV6Addr_t ipV6Addr;
        struct sockaddr_in6* ip6addr = (struct sockaddr_in6*)input;
        memcpy(ipV6Addr, &ip6addr->sin6_addr, PAL_IPV6_ADDRESS_SIZE);
        result = pal_setSockAddrIPV6Addr(out, ipV6Addr);
        if (result == PAL_SUCCESS)
        {
            result = pal_setSockAddrPort(out, PAL_NTOHS(ip6addr->sin6_port));
        }
        *length = sizeof(struct sockaddr_in6);
        found = true;
    }
#endif // PAL_SUPPORT_IP_V6

    if (false == found)
    { // we got unspeicified in one of the tests, so Don't fail , but don't translate address.  // re-chcking
        result = PAL_ERR_SOCKET_INVALID_ADDRESS_FAMILY;
    }

    return result;
}

palStatus_t pal_plat_bind(palSocket_t socket, palSocketAddress_t* myAddress, palSocketLength_t addressLength)
{
    int result = PAL_SUCCESS;
    int res = 0;
    struct sockaddr_in internalAddr = {0} ;
    
    result = pal_plat_SockAddrToSocketAddress(myAddress, (struct sockaddr *)&internalAddr);
    if (result == PAL_SUCCESS)
    {
        res = bind((intptr_t)socket, (struct sockaddr *)&internalAddr, addressLength);
        if (res == -1)
        {
            result = translateErrorToPALError(errno);
        }
    }

    return result;
}


palStatus_t pal_plat_receiveFrom(palSocket_t socket, void* buffer, size_t length, palSocketAddress_t* from, palSocketLength_t* fromLength, size_t* bytesReceived)
{
    palStatus_t result = PAL_SUCCESS;
    ssize_t res;
    struct sockaddr_in internalAddr;
    socklen_t addrlen;

    clearSocketFilter((intptr_t)socket);
    addrlen = sizeof(struct sockaddr_in);
    res = recvfrom((intptr_t)socket, buffer, length, 0 ,(struct sockaddr *)&internalAddr, &addrlen);
    if(res == -1)
    {
        result = translateErrorToPALError(errno);
    }
    else // only return address / bytesReceived in case of success
    {
        if ((NULL != from) && (NULL != fromLength))
        {
            result = pal_plat_socketAddressToPalSockAddr((struct sockaddr *)&internalAddr, from, fromLength);
        }
        *bytesReceived = res;
    }

    return result;
}

palStatus_t pal_plat_sendTo(palSocket_t socket, const void* buffer, size_t length, const palSocketAddress_t* to, palSocketLength_t toLength, size_t* bytesSent)
{
    palStatus_t result = PAL_SUCCESS;
    ssize_t res;

    clearSocketFilter((intptr_t)socket);
    res = sendto((intptr_t)socket, buffer, length, 0, (struct sockaddr *)to, toLength);
    if(res == -1)
    {
        result = translateErrorToPALError(errno);
    }
    else
    {
        *bytesSent = res;
    }

    return result;
}

palStatus_t pal_plat_close(palSocket_t* socket)
{
    palStatus_t result = PAL_SUCCESS;
    
    return result;
}

palStatus_t pal_plat_getNumberOfNetInterfaces( uint32_t* numInterfaces)
{        
    *numInterfaces = s_pal_numberOFInterfaces;
    return PAL_SUCCESS;
}

palStatus_t pal_plat_getNetInterfaceInfo(uint32_t interfaceNum, palNetInterfaceInfo_t * interfaceInfo)
{
    palStatus_t result = PAL_SUCCESS;    

    return result;
}


#if PAL_NET_TCP_AND_TLS_SUPPORT // functionality below supported only in case TCP is supported.
palStatus_t pal_plat_listen(palSocket_t socket, int backlog)
{
    palStatus_t result = PAL_SUCCESS;
    int res;

    res = listen((intptr_t)socket,backlog);
    if(res == -1)
    {
        result = translateErrorToPALError(errno);
    }
    return result;
}


palStatus_t pal_plat_accept(palSocket_t socket, palSocketAddress_t * address, palSocketLength_t* addressLen, palSocket_t* acceptedSocket)
{    
    intptr_t res = 0;
    palStatus_t result = PAL_SUCCESS;
    struct sockaddr_in internalAddr = {0} ;
    socklen_t internalAddrLen = sizeof(internalAddr);

    // XXX: the whole addressLen -concept is broken as the address is fixed size anyway.
    if (*addressLen < sizeof(palSocketAddress_t))
    {
        return PAL_ERR_SOCKET_INVALID_ADDRESS;
    }

    res = accept((intptr_t)socket,(struct sockaddr *)&internalAddr, &internalAddrLen);
    if(res == -1)
    {
        result = translateErrorToPALError(errno);
    }
    else
    {
        *acceptedSocket = (palSocket_t*)res;
        *addressLen = sizeof(palSocketAddress_t);
        result = pal_plat_socketAddressToPalSockAddr((struct sockaddr *)&internalAddr, address, &internalAddrLen);
    }

    return result;
}


palStatus_t pal_plat_connect(palSocket_t socket, const palSocketAddress_t* address, palSocketLength_t addressLen)
{
    int result = PAL_SUCCESS;
    int res;
    struct sockaddr_in internalAddr = {0} ;

    result = pal_plat_SockAddrToSocketAddress(address, (struct sockaddr *)&internalAddr);
    if (result == PAL_SUCCESS)
    {
        // clean filter to get the callback on first attempt
        clearSocketFilter((intptr_t)socket);

        res = connect((intptr_t)socket,(struct sockaddr *)&internalAddr, addressLen);
        if(res == -1)
        {
            result = translateErrorToPALError(errno);
        }
    }

    return result;
}

palStatus_t pal_plat_recv(palSocket_t socket, void *buffer, size_t len, size_t* recievedDataSize)
{
    palStatus_t result = PAL_SUCCESS;
    ssize_t res;

    clearSocketFilter((intptr_t)socket);
    res = recv((intptr_t)socket, buffer, len, 0);
    if(res ==  -1)
    {
        result = translateErrorToPALError(errno);
    }
    else
    {
        if (0 == res)
        {
            result = PAL_ERR_SOCKET_CONNECTION_CLOSED;
        }
        *recievedDataSize = res;
    }
    return result;
}

palStatus_t pal_plat_send(palSocket_t socket, const void *buf, size_t len, size_t *sentDataSize)
{
    palStatus_t result = PAL_SUCCESS;
    ssize_t res;

    clearSocketFilter((intptr_t)socket);

    res = send((intptr_t)socket, buf, len, 0);
    if(res == -1)
    {
        result = translateErrorToPALError(errno);
    }
    else
    {
        *sentDataSize = res;
    }

    return result;
}

#endif //PAL_NET_TCP_AND_TLS_SUPPORT


#if PAL_NET_ASYNCHRONOUS_SOCKET_API
palStatus_t pal_plat_asynchronousSocket(palSocketDomain_t domain, palSocketType_t type, bool nonBlockingSocket, uint32_t interfaceNum, palAsyncSocketCallback_t callback, void* callbackArgument, palSocket_t* socket)
{
    palStatus_t result = PAL_SUCCESS;

    return result;
}

#endif

#if PAL_NET_DNS_SUPPORT

palStatus_t pal_plat_getAddressInfo(const char *url, palSocketAddress_t *address, palSocketLength_t* length)
{
    palStatus_t result = PAL_SUCCESS;
    palSocketAddress_t localAddress = {0};
    palSocketAddress_t zeroAddress = {0};
    struct hostent * hn = NULL;
    struct sockaddr_in remotehost;
    unsigned int i = 0;    

    hn = gethostbyname(url);
    if(NULL == hn)
    {
        result = translateErrorToPALError(errno);
    }
    else
    {
        while( hn->h_addr_list[i] != NULL ) {            

            memcpy(&remotehost.sin_addr, hn->h_addr_list[i], hn->h_length);
            remotehost.sin_family = AF_INET;                    
        
            result = pal_plat_socketAddressToPalSockAddr((struct sockaddr *)&remotehost, &localAddress, length);

            if (0 == memcmp(localAddress.addressData, zeroAddress.addressData, PAL_NET_MAX_ADDR_SIZE) ) // invalid 0 address
            {
                result = PAL_ERR_SOCKET_DNS_ERROR;
            }
            else
            {
                *address = localAddress;
                result = PAL_SUCCESS;
                break;
            }
            i++;
        }
    }

    return result;
}

#endif
