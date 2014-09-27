#include "HttpServer.h"

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdio>
#include <cstring>

#include <stdexcept>
#include <string>
#include <system_error>

static void check(int result, const char* msg)
{
	if (result == -1)
	{
		throw std::system_error(errno, std::generic_category(), msg);
	}
}

HttpServer::HttpServer(uint16_t port)
{
	addrinfo hints = {};
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;
	
	char buf[8];
	snprintf(buf, 8, "%hu", port);
	printf("Binding to port %s\n", buf);
	
	addrinfo* result = nullptr;
	int err = getaddrinfo(nullptr, buf, &hints, &result);
	if (err != 0)
	{
		throw std::runtime_error(std::string("getaddrinfo: ") + gai_strerror(err));
	}
	
	int sock;
	try
	{
		addrinfo* rp;
		std::string reasons;
		for (rp = result; rp != nullptr; rp = rp->ai_next)
		{
			sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
			if (sock == -1)
			{
				reasons += "\n    socket: ";
				reasons += strerror(errno);
				continue;
			}
			
			if (bind(sock, rp->ai_addr, rp->ai_addrlen) == 0)
			{
				break;
			}
			else
			{
				char hostBuf[NI_MAXHOST];
				char servBuf[NI_MAXSERV];
				
				if ((err = getnameinfo(rp->ai_addr, rp->ai_addrlen, hostBuf, sizeof(hostBuf),
					servBuf, sizeof(servBuf), NI_NUMERICHOST | NI_NUMERICSERV)) != 0)
				{
					throw std::runtime_error(std::string("getnameinfo: ") + gai_strerror(err));
				}
				
				reasons += "\n    bind: (";
				if (rp->ai_family == AF_INET6)
				{
					reasons += '[';
					reasons += hostBuf;
					reasons += ']';
				}
				else
				{
					reasons += hostBuf;
				}
				
				reasons += ":";
				reasons += servBuf;
				reasons += "): ";
				reasons += strerror(errno);
			}
			
			check(close(sock), "close");
		}
	
		if (rp == nullptr)
		{
			throw std::runtime_error("Could not bind to any connection:" + reasons);
		}
		else
		{
			char hostBuf[NI_MAXHOST];
			char servBuf[NI_MAXSERV];
			
			if ((err = getnameinfo(rp->ai_addr, rp->ai_addrlen, hostBuf, sizeof(hostBuf),
				servBuf, sizeof(servBuf), NI_NUMERICHOST | NI_NUMERICSERV)) != 0)
			{
				throw std::runtime_error(std::string("getnameinfo: ") + gai_strerror(err));
			}
				
			if (rp->ai_family == AF_INET6)
			{
				printf("Bound socket at [%s]:%s\n", hostBuf, servBuf);
			}
			else
			{
				printf("Bound socket at %s:%s\n", hostBuf, servBuf);
			}
		}
	}
	catch (...)
	{
		freeaddrinfo(result);
		throw;
	}
	
	freeaddrinfo(result);
	
	if (close(sock) == -1)
	{
		perror("close");
		return;
	}
}
