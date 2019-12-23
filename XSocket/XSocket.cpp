#include "XSocket.h"
#ifdef WIN32
#include <MSTcpIP.h>
#include <ws2tcpip.h>
#else
#include <netdb.h>
#endif//

namespace XSocket {

size_t Tick()
{
#ifdef WIN32
	return GetTickCount();
#else
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
#endif//
}

long Socket::Init()
{
#ifdef WIN32
	WORD	Version;
	WSADATA wsaData;
	int		err;

	Version = MAKEWORD(2, 2);
	err = ::WSAStartup(Version, &wsaData);
	if (err != 0)
	{
		return -1;
	}
#endif//
	return 0;
}

void Socket::Term()
{
#ifdef WIN32
	::WSACleanup();
#endif//
}

// double H2N(double n)
// {
// 	return htond(n);
// }

// double N2H(double n)
// {
// 	return ntohd(n);
// }

// float H2N(float n)
// {
// 	return htonf(n);
// }

// float N2H(float n)
// {
// 	return ntohf(n);
// }

// uint64_t H2N(uint64_t n)
// {
// 	return htonll(n);
// }

// uint64_t N2H(uint64_t n)
// {
// 	return ntohll(n);
// }

u_long Socket::H2N(u_long n)
{
	return htonl(n);
}

u_long Socket::N2H(u_long n)
{
	return ntohl(n);
}

u_short Socket::H2N(u_short n)
{
	return htons(n);
}

u_short Socket::N2H(u_short n)
{
	return ntohs(n);
}

u_long Socket::Ip2N(const char* ip)
{
	return inet_addr(ip);
}

const char* Socket::N2Ip(u_long ip)
{
	struct in_addr addr = {0};
	addr.s_addr = ip;
	return (::inet_ntoa(addr));
}

const char* Socket::Url2Ip(const char* Url)
{
	struct hostent * lphost = NULL;
	sockaddr_in addr = {0};
	if (::inet_addr(Url) == INADDR_NONE) {
		if ((lphost = ::gethostbyname(Url)) == NULL) {
			//无效的域名
			return "";
		}
		memcpy((char *)&addr.sin_addr,(char *)lphost->h_addr,lphost->h_length);
		return (N2Ip(addr.sin_addr.s_addr));
	}
	//本来就是IP地址
	return Url;
}

int Socket::IpStr2IpAddr(const char* ip, int af, void* p)
{
	return inet_pton(af, ip, p);
}

const char* Socket::IpAddr2IpStr(void* p, int af, char* ip, int ip_len)
{
	memset(ip, 0, ip_len);
	inet_ntop(af, p, ip, ip_len);
	return ip;
}

int Socket::GetAddrInfo( const char *hostname, const char *service, const struct addrinfo *hints, struct addrinfo **result)
{
	return ::getaddrinfo(hostname, service, hints, result);
}

void Socket::SetAddrPort(struct sockaddr * addr, u_short port)
{
	auto netType = addr->sa_family;
	if (netType == AF_INET) {
		struct sockaddr_in *v4sa = (struct sockaddr_in *)addr;
		v4sa->sin_port = htons(port);
	} else if (netType == AF_INET6) {
		struct sockaddr_in6 *v6sa = (struct sockaddr_in6 *)addr;
		v6sa->sin6_port = htons(port);
	}
}

// void Socket::FreeAddrInfo( const struct addrinfo *ai)
// {
// 	freeaddrinfo(ai);
// }

u_short Socket::SockAddr2Port(const SOCKADDR* lpSockAddr, int nSockAddrLen)
{
	switch (lpSockAddr->sa_family)
	{
	case AF_INET:
		return N2H(((SOCKADDR_IN*)lpSockAddr)->sin_port);
		break;
	case AF_INET6:
		return N2H(((SOCKADDR_IN6*)lpSockAddr)->sin6_port);
		break;
	default:
		break;
	}
	return 0;
}

const char* Socket::SockAddr2IpStr(const SOCKADDR* lpSockAddr, int nSockAddrLen, char* str, int len)
{
	switch (lpSockAddr->sa_family)
	{
	case AF_INET:
		return IpAddr2IpStr(&((SOCKADDR_IN*)lpSockAddr)->sin_addr, AF_INET, str, len);
		break;
	case AF_INET6:
		return IpAddr2IpStr(&((SOCKADDR_IN6*)lpSockAddr)->sin6_addr, AF_INET6, str, len);
		break;
	default:
		break;
	}
	return "";
}

const char* Socket::SockAddr2PortStr(const SOCKADDR* lpSockAddr, int nSockAddrLen, char* str, int len)
{
	switch (lpSockAddr->sa_family)
	{
	case AF_INET:
	{
		snprintf(str, len, "%d", N2H(((SOCKADDR_IN*)lpSockAddr)->sin_port));
	}
	break;
	case AF_INET6:
	{
		snprintf(str, len, "%s:%d", N2H(((SOCKADDR_IN6*)lpSockAddr)->sin6_port));
	}
	break;
	default:
	break;
	}
	return "";
}

const char* Socket::SockAddr2Str(const SOCKADDR* lpSockAddr, int nSockAddrLen, char* str, int len)
{
	char ip[64] = {0};
	u_short port = 0;
	switch (lpSockAddr->sa_family)
	{
	case AF_INET:
	{
		IpAddr2IpStr(&((SOCKADDR_IN*)lpSockAddr)->sin_addr, AF_INET, ip, 64);
		port = N2H(((SOCKADDR_IN*)lpSockAddr)->sin_port);
		snprintf(str, len, "%s:%d", ip, port);
	}
	break;
	case AF_INET6:
	{
		IpAddr2IpStr(&((SOCKADDR_IN6*)lpSockAddr)->sin6_addr, AF_INET6, ip, 64);
		port = N2H(((SOCKADDR_IN6*)lpSockAddr)->sin6_port);
		snprintf(str, len, "%s:%d", ip, port);
	}
	break;
	default:
	break;
	}
	return "";
}

const char* Socket::Url2IpStr(const char* url, char* str, int len)
{
	struct addrinfo ai = {0}, *ai_res = nullptr;
	ai.ai_family = PF_UNSPEC;
	ai.ai_socktype = SOCK_STREAM;
#ifdef WIN32
	ai.ai_flags = 0;
#else
	ai.ai_flags = AI_DEFAULT;
#endif
	ai.ai_flags = AI_PASSIVE;
	int err = GetAddrInfo(url, nullptr, &ai, &ai_res);
	if(err) {
		return "";
	}
	return SockAddr2IpStr(ai_res->ai_addr, ai_res->ai_addrlen, str, len);
}

SOCKET Socket::Create(int nSockAf /* =AF_INET */, int nSockType /* = SOCK_STREAM */, int nSockProtocol /* = 0 */)
{
	return socket(nSockAf, nSockType, nSockProtocol);
}

bool Socket::IsSocket(SOCKET Sock)
{ 
	return Sock != 0 && Sock != INVALID_SOCKET;
}

int Socket::ShutDown(SOCKET Sock, int nHow)
{
	return shutdown(Sock, nHow);
}

int Socket::Close(SOCKET Sock)
{
#ifdef WIN32
	return closesocket(Sock);
#else //LINUX
	//套节子的判断 应该是 > 0 因为合法的套节子
	//在LINUX下面是 > 0的
	if(Sock>0) {
		close(Sock);
	}
#endif//
}

SOCKET Socket::Accept(SOCKET Sock, SOCKADDR* lpSockAddr, int* lpSockAddrLen)
{
#ifdef WIN32
	return accept(Sock, lpSockAddr, lpSockAddrLen);
#else
	return accept(Sock,(struct sockaddr *)lpSockAddr,(socklen_t *)lpSockAddrLen);
#endif//
}

int Socket::Bind(SOCKET Sock, const SOCKADDR* lpSockAddr, int nSockAddrLen)
{
	return bind(Sock, lpSockAddr, nSockAddrLen);
}

int Socket::Connect(SOCKET Sock, const SOCKADDR* lpSockAddr, int nSockAddrLen)
{
	return connect(Sock, lpSockAddr, nSockAddrLen);
}

int Socket::Listen(SOCKET Sock, int nConnectionBacklog/* = 5*/)
{
	return listen(Sock, nConnectionBacklog);
}

int Socket::Send(SOCKET Sock, const char* lpBuf, int nBufLen, int nFlags)
{
	return send(Sock, lpBuf, nBufLen, nFlags);
}

int Socket::Receive(SOCKET Sock, char* lpBuf, int nBufLen, int nFlags)
{
	return recv(Sock, lpBuf, nBufLen, nFlags);
}

//int SyncSend(SOCKET Sock, const char* lpBuf, int nBufLen, int nFlags)
//{
//	//if (MSG_PARTIAL)
//	int nBufSend = 0;
//	while(nBufSend < nBufLen)
//	{
//		int nSend = Send(Sock, lpBuf + nBufSend, nBufLen - nBufSend, nFlags);
//		if(nSend == SOCKET_ERROR) {
//			int err = GetLastError();
//			if(err == WSAEWOULDBLOCK) {
//				Sleep(1);
//				continue;
//			} else {
//				return SOCKET_ERROR;
//			}
//		}
//		nBufSend += nSend;
//	}
//	return nBufSend;
//}
//
//int SyncReceive(SOCKET Sock, char* lpBuf, int nBufLen, int nFlags)
//{
//	if(!(nFlags&MSG_WAITALL)) {
//		int nBufRecv = 0;
//		do
//		{
//			nBufRecv = Receive(Sock, lpBuf, nBufLen, nFlags);
//			if(nBufRecv == SOCKET_ERROR) {
//				int err = GetLastError();
//				if(err == WSAEWOULDBLOCK) {
//					Sleep(1);
//					continue;
//				} else {
//					return SOCKET_ERROR;
//				}
//			}
//			break;
//		} while(true);
//		return nBufRecv;
//	} else {
//		int nBufRecv = 0;
//		while(nBufRecv < nBufLen)
//		{
//			int nRecv = Receive(Sock, lpBuf + nBufRecv, nBufLen - nBufRecv, nFlags);
//			if(nRecv == SOCKET_ERROR) {
//				int Error = GetLastError();
//				if(Error == WSAEWOULDBLOCK) {
//					Sleep(1);
//					continue;
//				} else {
//					return Error;
//				}
//			}
//			nBufRecv += nRecv;
//		}
//		return nBufRecv;
//	}
//}

int Socket::SendTo(SOCKET Sock, const char* lpBuf, int nBufLen, const SOCKADDR* lpSockAddr, int nSockAddrLen, int nFlags)
{
	return sendto(Sock, lpBuf, nBufLen, nFlags, lpSockAddr, nSockAddrLen);
}

int Socket::ReceiveFrom(SOCKET Sock, char* lpBuf, int nBufLen, SOCKADDR* lpSockAddr, int* lpSockAddrLen, int nFlags)
{
#ifdef WIN32
	return recvfrom(Sock, lpBuf, nBufLen, nFlags, lpSockAddr, lpSockAddrLen);
#else
	return recvfrom(Sock, lpBuf, nBufLen, nFlags, lpSockAddr, (socklen_t*)lpSockAddrLen);
#endif//
}

int Socket::IOCtl(SOCKET Sock, long lCommand, u_long* lpArgument)
{
#ifdef WIN32
	return ioctlsocket(Sock, lCommand, lpArgument);
#else
	return fcntl(Sock, lCommand, *lpArgument);
#endif//
}

int Socket::IOCtl(SOCKET Sock, long lCommand, u_long Argument)
{
#ifdef WIN32
	return ioctlsocket(Sock, lCommand, &Argument);
#else
	return fcntl(Sock, lCommand, Argument);
#endif//
}

int Socket::GetSockOpt(SOCKET Sock, int nLevel, int nOptionName, void* lpOptionValue, int* lpOptionLen)
{
#ifdef WIN32
	return getsockopt(Sock, nLevel, nOptionName, (char*)lpOptionValue, lpOptionLen);
#else
	return getsockopt(Sock, nLevel, nOptionName, (char*)lpOptionValue, (socklen_t*)lpOptionLen);
#endif//
}

int Socket::GetSockOpt(SOCKET Sock, int nLevel, int nOptionName, void* lpOptionValue, int nOptionLen)
{
	return GetSockOpt(Sock, nLevel, nOptionName, lpOptionValue, &nOptionLen);
}

int Socket::SetSockOpt(SOCKET Sock, int nLevel, int nOptionName, const void* lpOptionValue, int nOptionLen)
{
	return setsockopt(Sock, nLevel, nOptionName, (char*)lpOptionValue, nOptionLen);
}

int Socket::SetSendTimeOut(SOCKET Sock, int TimeOut)
{
#ifdef WIN32
	return SetSockOpt(Sock, SOL_SOCKET, SO_SNDTIMEO, &TimeOut, sizeof(int));
#else
	struct timeval sttime;
	sttime.tv_sec = TimeOut/1000;
	sttime.tv_usec = 0;
	return SetSockOpt(Sock, SOL_SOCKET, SO_SNDTIMEO, (void*)&sttime, sizeof(sttime));
#endif//
}

int Socket::SetRecvTimeOut(SOCKET Sock, int TimeOut)
{
#ifdef WIN32
	return SetSockOpt(Sock, SOL_SOCKET, SO_RCVTIMEO, &TimeOut, sizeof(int));
#else
	struct timeval sttime;
	sttime.tv_sec = TimeOut/1000;
	sttime.tv_usec = 0;
	return SetSockOpt(Sock, SOL_SOCKET, SO_RCVTIMEO, (void*)&sttime, sizeof(sttime));
#endif//
}

int Socket::GetSendTimeOut(SOCKET Sock)
{
	int TimeOut = 0;
#ifdef WIN32
	if(SOCKET_ERROR != GetSockOpt(Sock, SOL_SOCKET, SO_SNDTIMEO, (void*)&TimeOut, sizeof(int))) {
		return TimeOut;
	}
#else
	struct timeval sttime = {0};
	if(SOCKET_ERROR != GetSockOpt(Sock, SOL_SOCKET, SO_SNDTIMEO, (void*)&sttime, sizeof(sttime))) {
		TimeOut = sttime.tv_sec * 1000;
		return TimeOut;
	}
#endif//
	return SOCKET_ERROR;
}

int Socket::GetRecvTimeOut(SOCKET Sock)
{
	int TimeOut = 0;
#ifdef WIN32
	if(SOCKET_ERROR != GetSockOpt(Sock, SOL_SOCKET, SO_RCVTIMEO, (void*)&TimeOut, sizeof(int))) {
		return TimeOut;
	}
#else
	struct timeval sttime = {0};
	if(SOCKET_ERROR != GetSockOpt(Sock, SOL_SOCKET, SO_RCVTIMEO, (void*)&sttime, sizeof(sttime))) {
		TimeOut = sttime.tv_sec * 1000;
		return TimeOut;
	}
#endif//
	return SOCKET_ERROR;
}

int Socket::SetKeepAlive(SOCKET Sock, u_long onoff, u_long time)
{
#ifdef WIN32
	if (SetSockOpt(Sock, SOL_SOCKET, SO_KEEPALIVE, &onoff,  sizeof onoff) == 0) { 
		struct tcp_keepalive kavars[1] = {
			onoff, time, time/2
		};
		DWORD ret = 0;
		return WSAIoctl(Sock, SIO_KEEPALIVE_VALS, kavars, sizeof kavars, NULL, sizeof (int), &ret, NULL, NULL);
	}
#else
	int val = onoff;
	//开启keepalive机制  
	if (SetSockOpt(Sock, SOL_SOCKET, SO_KEEPALIVE, &val, sizeof(val)) == 0) {
		/* Default settings are more or less garbage, with the keepalive time 
		 * set to 7200 by default on Linux. Modify settings to make the feature 
		 * actually useful. 
		 */  

		/* Send first probe after interval. */  
		val = time;  
		SetSockOpt(Sock, IPPROTO_TCP, TCP_KEEPIDLE, &val, sizeof(val));

		/* Send next probes after the specified interval. Note that we set the 
		 * delay as interval / 3, as we send three probes before detecting 
		 * an error (see the next setsockopt call). 
		 */  
		val = time/3;  
		if (val == 0) {
			val = 1;
		}
		SetSockOpt(Sock, IPPROTO_TCP, TCP_KEEPINTVL, &val, sizeof(val));
	
		/* Consider the socket in error state after three we send three ACK 
		 * probes without getting a reply. 
		 */  
		val = 3;  
		SetSockOpt(Sock, IPPROTO_TCP, TCP_KEEPCNT, &val, sizeof(val));

		return 0;
	}  
#endif//
	return SOCKET_ERROR;
}

int Socket::GetPeerName(SOCKET Sock, SOCKADDR* lpSockAddr, int* lpSockAddrLen)
{
#ifdef WIN32
	return getpeername(Sock, lpSockAddr, lpSockAddrLen);
#else
	return getpeername(Sock, lpSockAddr, (socklen_t*)lpSockAddrLen);
#endif//
}

int Socket::GetSockName(SOCKET Sock, SOCKADDR* lpSockAddr, int* lpSockAddrLen)
{
#ifdef WIN32
	return getsockname(Sock, lpSockAddr, lpSockAddrLen);
#else
	return getsockname(Sock, lpSockAddr, (socklen_t*)lpSockAddrLen);
#endif//
}

int Socket::GetLastError()
{
#ifdef WIN32
	return WSAGetLastError();
#else
	return (unsigned long)errno;
#endif//
}

void Socket::SetLastError(int nError)
{
#ifdef WIN32
	WSASetLastError(nError);
#else
	errno = nError;
#endif//
}

int Socket::GetErrorMessage(int nError, char* lpszMessage, int nMessageLen)
{
#ifdef WIN32
	DWORD dwError = nError;
	if (lpszMessage == NULL || nMessageLen == 0) {
		return SOCKET_ERROR;
	}
	HLOCAL hLocal = NULL;
	if (!::FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL, dwError, LANG_NEUTRAL, (LPSTR)&hLocal, 0, NULL)) {
		HMODULE hDll = ::LoadLibraryExA("netmsg.dll", NULL, DONT_RESOLVE_DLL_REFERENCES);
		if (hDll != NULL) {
			::FormatMessageA(FORMAT_MESSAGE_FROM_HMODULE|FORMAT_MESSAGE_FROM_SYSTEM,
				hDll, dwError, LANG_NEUTRAL, (LPSTR)&hLocal, 0, NULL);
			::FreeLibrary(hDll);
		}
	}
	if (hLocal) {
		strncpy(lpszMessage, (LPSTR)::LocalLock(hLocal), nMessageLen-1);
		lpszMessage[nMessageLen-1] = 0;
		::LocalFree(hLocal);
		return nMessageLen;
	}
#else
	snprintf(lpszMessage, nMessageLen, "%s",strerror(nError));
#endif//
	return SOCKET_ERROR;
}

int Socket::GetErrorMessage(int nError, wchar_t* lpszMessage, int nMessageLen)
{
#ifdef WIN32
	DWORD dwError = nError;
	if (lpszMessage == NULL || nMessageLen == 0) {
		return SOCKET_ERROR;
	}
	HLOCAL hLocal = NULL;
	if (!::FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL, dwError, LANG_NEUTRAL, (LPWSTR)&hLocal, 0, NULL)) {
			HMODULE hDll = ::LoadLibraryExW(L"netmsg.dll", NULL, DONT_RESOLVE_DLL_REFERENCES);
			if (hDll != NULL) {
				::FormatMessageW(FORMAT_MESSAGE_FROM_HMODULE|FORMAT_MESSAGE_FROM_SYSTEM,
					hDll, dwError, LANG_NEUTRAL, (LPWSTR)&hLocal, 0, NULL);
				::FreeLibrary(hDll);
			}
	}
	if (hLocal) {
		wcsncpy(lpszMessage, (LPWSTR)::LocalLock(hLocal), nMessageLen-1);
		lpszMessage[nMessageLen-1] = 0;
		::LocalFree(hLocal);
		return nMessageLen;
	}
#else
	char* str = strerror(nError);
	mbstowcs(lpszMessage, str, strlen(str)+1);
#endif//
	return SOCKET_ERROR;
}

}
