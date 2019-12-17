// server.cpp : 定义控制台应用程序的入口点。
//

#include "../../samples.h"
#include "../../../XSocket/XSocketImpl.h"
#ifdef USE_EPOLL
#include "../../../XSocket/XEPoll.h"
#elif defined(USE_IOCP)
#include "../../../XSocket/XCompletionPort.h"
#endif//

class worker;

class Event
{
public:
	worker* dst = nullptr;
	int id;
	std::string buf;
#ifdef USE_UDP
	SOCKADDR_IN addr;
#endif
	int flags;

	Event() {}
#ifndef USE_UDP
	Event(worker* d, int id, const char* buf, int len, int flag):dst(d),id(id),buf(buf,len),flags(flag){}
#else
	Event(worker* d, int id, const char* buf, int len, const SOCKADDR_IN& addr, int flag):dst(d),id(id),buf(buf,len),addr(addr),flags(flag){}
#endif

	// inline int get_id() { return evt; }
	// inline const char* get_data() { return data.c_str(); }
	// inline int get_datalen() { return data.size(); }
	// inline int get_flags() { return flags; }
};
typedef XSocket::SimpleEventServiceT<XSocket::DelayEventServiceT<Event,XSocket::ThreadService>> WorkService;
//typedef XSocket::ThreadService WorkService;

#ifndef USE_UDP
#ifdef USE_EPOLL
typedef XSocket::EPollSocketSetT<WorkService,worker,DEFAULT_FD_SETSIZE> WorkSocketSet;
#elif defined(USE_IOCP)
typedef XSocket::CompletionPortSocketSetT<WorkService,worker,DEFAULT_FD_SETSIZE> WorkSocketSet;
#else
typedef XSocket::SelectSocketSetT<WorkService,worker,DEFAULT_FD_SETSIZE> WorkSocketSet;
#endif//
#endif//USE_UDP

#ifndef USE_UDP
class worker
#ifdef USE_EPOLL
	: public XSocket::SocketExImpl<worker,XSocket::SimpleEvtSocketT<XSocket::WorkSocketT<XSocket::EPollSocketT<WorkSocketSet,XSocket::SocketEx>>>>
#elif defined(USE_IOCP)
	: public XSocket::SocketExImpl<worker,XSocket::SimpleEvtSocketT<XSocket::WorkSocketT<XSocket::CompletionPortSocketT<WorkSocketSet,XSocket::SocketEx>>>>
#else
	: public XSocket::SocketExImpl<worker,XSocket::SimpleEvtSocketT<XSocket::WorkSocketT<XSocket::SelectSocketT<WorkSocketSet,XSocket::SocketEx>>>>
#endif
{
#ifdef USE_EPOLL
	typedef XSocket::SocketExImpl<worker,XSocket::SimpleEvtSocketT<XSocket::WorkSocketT<XSocket::EPollSocketT<WorkSocketSet,XSocket::SocketEx>>>> Base;
#elif defined(USE_IOCP)
	typedef XSocket::SocketExImpl<worker,XSocket::SimpleEvtSocketT<XSocket::WorkSocketT<XSocket::CompletionPortSocketT<WorkSocketSet,XSocket::SocketEx>>>> Base;
#else
	typedef XSocket::SocketExImpl<worker,XSocket::SimpleEvtSocketT<XSocket::WorkSocketT<XSocket::SelectSocketT<WorkSocketSet,XSocket::SocketEx>>>> Base;
#endif
protected:
	
public:
	worker()
	{
		
	}

	~worker() 
	{
		
	}

public:
	inline void PostBuf(const char* lpBuf, int nBufLen, int nFlags = 0)
	{
		Post(Event(this,FD_WRITE,lpBuf,nBufLen,nFlags));
	}

	virtual void OnEvent(const Event& evt)
	{
		if(evt.id == FD_WRITE) {
			SendBuf(evt.buf.c_str(),evt.buf.size(),evt.flags);
		}
	}
protected:
	//
	virtual void OnIdle(int nErrorCode)
	{
		/*char lpBuf[DEFAULT_BUFSIZE+1];
		int nBufLen = 0;
		int nFlags = 0;
		nBufLen = Receive(lpBuf,DEFAULT_BUFSIZE,&nFlags);
		if (nBufLen<=0) {
			return;
		}
		lpBuf[nBufLen] = 0;
		PRINTF("%s\n", lpBuf);
		PRINTF("echo:%s\n", lpBuf);
		Send(lpBuf,nBufLen);*/
	}

	virtual void OnRecvBuf(const char* lpBuf, int nBufLen, int nFlags)
	{
		PostBuf(lpBuf,nBufLen,0);
		Base::OnRecvBuf(lpBuf,nBufLen,nFlags);
	}

};

class server 
	: public XSocket::SelectServerT<XSocket::ThreadService,XSocket::SocketExImpl<server,XSocket::ListenSocketT<XSocket::SocketEx>>,WorkSocketSet>
{
	typedef XSocket::SelectServerT<XSocket::ThreadService,XSocket::SocketExImpl<server,XSocket::ListenSocketT<XSocket::SocketEx>>,WorkSocketSet> Base;
public:
	server(int nMaxSocketCount = DEFAULT_MAX_FD_SETSIZE):Base(nMaxSocketCount)
	{

	}
	
	bool OnChar(char c)
	{
		switch(c)
		{
		case 'x':
		case 'X':
			printf("server worker count is [%d]\n", GetSocketCount());
			break;
		case 'q':
		case 'Q':
			return false;
			break;
		}
		return true;
	}
};

#else
class server : public XSocket::SocketExImpl<server,XSocket::SelectUdpServerT<XSocket::ThreadService,XSocket::SimpleUdpSocketT<XSocket::SocketEx>>>
{
	typedef XSocket::SocketExImpl<server,XSocket::SelectUdpServerT<XSocket::ThreadService,XSocket::SimpleUdpSocketT<XSocket::SocketEx>>> Base;
protected:
	std::string addr_;
	u_short port_;
public:

	bool Start(const char* address, u_short port)
	{
		addr_ = address;
		port_ = port;
		if(!Base::Start()) {
			return false;
		}
		return true;
	}

protected:
	//
	virtual bool OnInit()
	{
		bool ret = Base::OnInit();
		if(!ret) {
			return false;
		}
		if(port_ <= 0) {
			return false;
		}
		Open(AF_INET,SOCK_DGRAM);
		SetSockOpt(SOL_SOCKET, SO_REUSEADDR, 1);
		Bind(addr_.c_str(), port_);
		Select(FD_READ);
	#ifdef WIN32
		IOCtl(FIONBIO, 1);//设为非阻塞模式
	#else
		int flags = IOCtl(F_GETFL,(u_long)0); 
		IOCtl(F_SETFL, (u_long)(flags|O_NONBLOCK)); //设为非阻塞模式
		//IOCtl(F_SETFL, (u_long)(flags&~O_NONBLOCK)); //设为阻塞模式
	#endif//
		return true;
	}

	virtual void OnTerm()
	{
		//服务结束运行，释放资源
		if(Base::IsSocket()) {
#ifndef WIN32
			Base::ShutDown();
#endif
			Base::Trigger(FD_CLOSE, 0);
		}
	}

	virtual void OnRecvBuf(const char* lpBuf, int nBufLen, const SockAddrType & SockAddr)
	{
		PRINTF("%.*s\n", nBufLen, lpBuf);
		PRINTF("echo[(%s:%d)]:%.*s\n",XSocket::N2Ip(SockAddr.sin_addr.s_addr),XSocket::N2H(SockAddr.sin_port),nBufLen, lpBuf);
		SendBuf(lpBuf,nBufLen,SockAddr,XSocket::SOCKET_PACKET_FLAG_TEMPBUF);
		Base::OnRecvBuf(lpBuf, nBufLen, SockAddr);
	}
};
#endif//USE_UDP

#ifdef WIN32
int _tmain(int argc, _TCHAR* argv[])
#else
int main()
#endif//
{
	XSocket::InitNetEnv();

	server *s = new server();
	s->Start(DEFAULT_IP, DEFAULT_PORT);
	getchar();
	s->Stop();
	delete s;

	XSocket::ReleaseNetEnv();

	return 0;
}

