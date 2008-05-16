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

typedef void (*accept_callback_t)(SOCKET client_sock, const char *client_ip, u_short client_port);

/* run accepting loop.
 * accepts connections on accept_sock till hExit is signaled
 * upon accepting the connection callback is called
 */
int nw_accept_loop(SOCKET accept_sock, HANDLE h_exit, accept_callback_t callback);

/* 
 * install console handler reacting to various break events by signaling an event 
 * returns event handle
 */
HANDLE nw_install_break_handler(void);

#endif
