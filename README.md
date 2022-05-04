# XSocket
## 简单的Modern C++ Socket跨平台可伸缩实现

* 平台：支持Windows、Linux、Mac OS、Android、iOS等全平台
* 服务：支持select/完成端口/epoll服务模型
* 套接字：支持Tcp/Udp的select/完成端口/epoll模型，全面支持IPV4、IPV6
* 协议：支持自定义协议适配，只需实现Parse接口
* 定制：支持服务、套接字、协议层次的定制
* SSL：支持OpenSSL非阻塞SSL通信
* 代理：支持SOCK4/4a/SOCK5/Http代理
* DNS：支持异步DNS
* HTTP：支持Http/WebSocket协议
* HTTP2: 支持Http /2协议
* QUIC: 支持Quic协议
* HTTP3: 支持Http Quic(Http /3)协议

플랫폼: Windows, Linux, Mac OS, Android, iOS 등과 같은 모든 플랫폼을 지원합니다.
서비스: 선택/완료 포트/epoll 서비스 모델 지원
소켓: Tcp/Udp 선택/완전 포트/epoll 모델 지원, IPV4, IPV6 완전 지원
프로토콜: 맞춤형 프로토콜 적응 지원, Parse 인터페이스만 구현하면 됨
사용자 정의: 서비스, 소켓 및 프로토콜 수준의 사용자 정의 지원
SSL: OpenSSL 비차단 SSL 통신 지원
프록시: SOCK4/4a/SOCK5/Http 프록시 지원
DNS: 비동기 DNS 지원
HTTP: Http/WebSocket 프로토콜 지원
HTTP2: HTTP /2 프로토콜 지원
QUIC: 빠른 프로토콜 지원
HTTP3: Http Quick(Http /3) 프로토콜 지원
