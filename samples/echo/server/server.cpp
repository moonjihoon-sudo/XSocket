// server.cpp : 定义控制台应用程序的入口点。
//

#include "../../samples.h"
#include "../../../XSocket/XSocketImpl.h"
#ifdef USE_EPOLL
#include "../../../XSocket/XEPoll.h"
#elif defined(USE_IOCP)
#include "../../../XSocket/XCompletionPort.h"
#endif//
using namespace XSocket;

#ifndef USE_UDP

class worker;

class Event
{
public:
	worker* dst = nullptr;
	int id;
	std::string buf;
#ifdef USE_UDP
	SockAddrType addr;
#endif
	int flags;

	Event() {}
#ifndef USE_UDP
	Event(worker* d, int id, const char* buf, int len, int flag):dst(d),id(id),buf(buf,len),flags(flag){}
#else
	Event(worker* d, int id, const char* buf, int len, const SockAddrType& addr, int flag):dst(d),id(id),buf(buf,len),addr(addr),flags(flag){}
#endif

	// inline int get_id() { return evt; }
	// inline const char* get_data() { return data.c_str(); }
	// inline int get_datalen() { return data.size(); }
	// inline int get_flags() { return flags; }
};
typedef SimpleEventServiceT<DelayEventServiceT<Event,ThreadService>> WorkService;
//typedef ThreadService WorkService;

class WorkSocket;
class WorkSocketSet;

class WorkSocketSet : public
#ifdef USE_EPOLL
EPollSocketSetT<WorkService,WorkSocket,DEFAULT_FD_SETSIZE>
#elif defined(USE_IOCP)
CompletionPortSocketSetT<WorkService,WorkSocket,DEFAULT_FD_SETSIZE>
#else
SelectSocketSetT<WorkService,WorkSocket,DEFAULT_FD_SETSIZE>
#endif//
{

};

class WorkSocket : public
#ifdef USE_EPOLL
EPollSocketT<WorkSocketSet,SocketEx,SockAddrType>
#elif defined(USE_IOCP)
CompletionPortSocketT<WorkSocketSet,SocketEx,SockAddrType>
#else
SelectSocketT<WorkSocketSet,SocketEx,SockAddrType>
#endif//
{

};

class worker : public SocketExImpl<worker,SimpleEvtSocketT<SimpleSocketT<WorkSocketT<WorkSocket>>>>
{
	typedef SocketExImpl<worker,SimpleEvtSocketT<SimpleSocketT<WorkSocketT<WorkSocket>>>> Base;
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
#if 0
	: public SelectServerT<ThreadService,SocketExImpl<server,ListenSocketT<SocketEx>>,WorkSocketSet>
#else
	: public SocketManagerT<WorkSocketSet>
#endif//
{
#if 0
	typedef SelectServerT<ThreadService,SocketExImpl<server,ListenSocketT<SocketEx>>,WorkSocketSet> Base;
#else
	typedef SocketManagerT<WorkSocketSet> Base;
protected:
	class listener : public SocketExImpl<listener,SimpleEvtSocketT<ListenSocketT<WorkSocket>>>
	, public std::enable_shared_from_this<listener>
	{
	protected:
		server* srv_;
	public:
		listener(server* srv):srv_(srv)
		{
		}

	protected:
		//
		virtual void OnAccept(const SOCKADDR* lpSockAddr, int nSockAddrLen, SOCKET Sock) 
		{
				//测试下还能不能再接收SOCKET
				if(srv_->AddSocket(NULL) < 0) {
					PRINTF("The connection was refused by the computer running select server because the maximum number of sessions has been exceeded.\n");
					XSocket::Socket::Close(Sock);
					return;
				}
				std::shared_ptr<worker> sock_ptr = std::make_shared<worker>();
				sock_ptr->Attach(Sock,SOCKET_ROLE_WORK);
				
	#ifdef WIN32
				sock_ptr->IOCtl(FIONBIO, 1);//设为非阻塞模式
	#else
				int flags = sock_ptr->IOCtl(F_GETFL,(u_long)0); 
				sock_ptr->IOCtl(F_SETFL, (u_long)(flags|O_NONBLOCK)); //设为非阻塞模式
				//sock_ptr->IOCtl(F_SETFL, (u_long)(flags&~O_NONBLOCK)); //设为阻塞模式
	#endif//
				int pos = srv_->AddSocket(sock_ptr, FD_READ|FD_OOB);
				if(pos >= 0) {
					//
				} else {
					PRINTF("The connection was refused by the computer running select server because the maximum number of sessions has been exceeded.\n");
					sock_ptr->Trigger(FD_CLOSE, 0);
				}
		}
	};
#endif
public:
#if 0
	server(int nMaxSocketCount = DEFAULT_MAX_FD_SETSIZE):Base(nMaxSocketCount)
#else
	server(int nMaxSocketCount = DEFAULT_MAX_FD_SETSIZE):Base((nMaxSocketCount+WorkSocketSet::GetMaxSocketCount()-1)/WorkSocketSet::GetMaxSocketCount())
#endif
	{

	}

#if 1
	bool Start(const char* address, u_short port)
	{
		bool ret = Base::Start();
		if(!ret) {
			return false;
		}

		std::shared_ptr<listener> sock_ptr = std::make_shared<listener>(this);
		sock_ptr->Open();
		sock_ptr->SetSockOpt(SOL_SOCKET, SO_REUSEADDR, 1);
		sock_ptr->Bind(address, port);
		sock_ptr->Listen();
		AddSocket(sock_ptr, FD_ACCEPT);

		return true;
	}
#else
protected:
	//
	virtual bool OnInit()
	{
		if(port_ <= 0) {
			return false;
		}
		Base::Open();
		Base::SetSockOpt(SOL_SOCKET, SO_REUSEADDR, 1);
		Base::Bind(address_.c_str(), port_);
		Base::Listen();
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
#endif//
};

#else

class server : public SocketExImpl<server,SelectUdpServerT<ThreadService,SimpleUdpSocketT<WorkSocketT<SocketEx>>>>
{
	typedef SocketExImpl<server,SelectUdpServerT<ThreadService,SimpleUdpSocketT<WorkSocketT<SocketEx>>>> Base;
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
		PRINTF("echo[(%s:%d)]:%.*s\n",N2Ip(SockAddr.sin_addr.s_addr),N2H(SockAddr.sin_port),nBufLen, lpBuf);
		SendBuf(lpBuf,nBufLen,SockAddr,SOCKET_PACKET_FLAG_TEMPBUF);
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
	Socket::Init();

	server *s = new server();
	s->Start(DEFAULT_IP, DEFAULT_PORT);
	getchar();
	s->Stop();
	delete s;

	Socket::Term();

	return 0;
}

