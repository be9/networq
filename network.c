#include "network.h"

/* initialize network */
int nw_init(void)
{
	WSADATA wsadata;
	int err = WSAStartup(MAKEWORD(2,2), &wsadata);

	if (err != 0)
		return err;
	
	if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wVersion) != 2) {
	    /* Tell the user that we could not find a usable */
    	/* WinSock DLL.                                  */
	    WSACleanup();
	    
		return -1;
	}
	
	return 0;
}

/* shutdown */
void nw_shutdown(void)
{
	WSACleanup();
}

/* create socket and set to listening mode. returns 0 on success */
int nw_listen(SOCKET *psock, int port)
{
	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_in service;

  	if (s == INVALID_SOCKET)
		return WSAGetLastError();

	service.sin_family = AF_INET;
	service.sin_addr.s_addr = INADDR_ANY;
	service.sin_port = htons(port);

	if (bind(s, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR ||
			listen(s, SOMAXCONN) == SOCKET_ERROR) {
	    int err = WSAGetLastError();

    	closesocket(s);
		
		return err;
	}

	*psock = s;
	return 0;
}

/* close a socket */
void nw_closesocket(SOCKET s)
{
	closesocket(s);
}
