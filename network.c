#include "network.h"
#include <windows.h>
#include <stdio.h>

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
	struct sockaddr_in service;

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

int nw_accept_loop(SOCKET accept_sock, HANDLE h_exit, accept_callback_t callback)
{
	HANDLE accept_event = CreateEvent(NULL, FALSE, FALSE, NULL);
	int ret;
	
	for (;;) {
		HANDLE h[2];
		DWORD res;

		/* set nonblocking mode on a socket and tie it to an event object */
		if (WSAEventSelect(accept_sock, accept_event, FD_ACCEPT) == SOCKET_ERROR) {
			CloseHandle(accept_event);
			return WSAGetLastError();
		}
	
		h[0] = accept_event;
		h[1] = h_exit;

		res = WaitForMultipleObjects(2, h, FALSE, INFINITE);

		if (res == WAIT_OBJECT_0) {
			/* connect request on socket */
			struct sockaddr_in sin_remote;
			int size = sizeof (sin_remote);

			SOCKET sd = accept(accept_sock, (struct sockaddr *)&sin_remote, &size);

			if (sd != INVALID_SOCKET) {
				/* set socket to a non-blocking mode */
				u_long iMode = 0;
				
				WSAEventSelect(sd, 0, 0);
				ioctlsocket(sd, FIONBIO, &iMode);
				
				callback(sd, inet_ntoa(sin_remote.sin_addr), ntohs(sin_remote.sin_port));
				continue;
			}
        
			/* error */
			ret = WSAGetLastError();
			
			break;
		} else if (res == WAIT_OBJECT_0+1) {
			/* exit normally */
			ret = 0;
			break;
		} else {
			/* some strange error from WaitForMultipleObjects() */
			ret = -1;
			break;
		}
	}

	CloseHandle(accept_event);

	return ret;
}

static HANDLE h_exit = NULL;
static BOOL ctrl_handler(DWORD fdwCtrlType)
{
	switch (fdwCtrlType) { 
		case CTRL_C_EVENT: 				/* Ctrl-C pressed */
		case CTRL_CLOSE_EVENT: 			/* Close button on window pressed */
		case CTRL_BREAK_EVENT:			/* Ctrl-Break pressed */
		case CTRL_LOGOFF_EVENT: 		/* User is to log off from system */
		case CTRL_SHUTDOWN_EVENT: 		/* System is shut down */
			SetEvent(h_exit);
			/* we handled that */
			return TRUE;

		default: 
			return FALSE;
	}
}	

HANDLE nw_install_break_handler(void)
{
	/* skip if already installed */
	if (h_exit != NULL)
		return h_exit;

	h_exit = CreateEvent(NULL, FALSE, FALSE, NULL);

	if (h_exit == NULL)
		return NULL;

	if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)ctrl_handler, TRUE)) {
		/* could not set handler */
		CloseHandle(h_exit);
		h_exit = NULL;
	}

	return h_exit;
}
