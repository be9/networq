#ifndef __network_h
#define __network_h

#include <Winsock2.h>

/* initialize network. returns 0 on success */
int nw_init(void);

/* shutdown */
void nw_shutdown(void);

/* create socket and set to listening mode. returns 0 on success */
int nw_listen(SOCKET *sock, int port);

/* close a socket */
void nw_closesocket(SOCKET s);

#endif
