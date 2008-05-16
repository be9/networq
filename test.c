#include <stdio.h>
#include <process.h>
#include <stdlib.h>
#include "network.h"

HANDLE *h_threads = 0;
int n_threads = 0, n_allocated = 0;

unsigned __stdcall sock_thread(void *arg)
{
	SOCKET s = (SOCKET)arg;
	char buf[1024];
	int len;

	fprintf(stderr, "In thread\n");

	while ((len = recv(s, buf, sizeof (buf), 0)) > 0) {
		send(s, buf, len, 0);
	}

	if (len == SOCKET_ERROR) {
		fprintf(stderr, "Socket error %d!\n", WSAGetLastError());
	} else {
		fprintf(stderr, "Connection gracefully closed!\n");
	}

	closesocket(s);

	return 0;
}	

void accept_callback(SOCKET client_sock, const char *client_ip, u_short client_port)
{
	HANDLE hthr;

	fprintf(stderr, "Got a connection from %s (port %d)\n", client_ip, client_port);

	hthr = (HANDLE)_beginthreadex(NULL, 0, &sock_thread, (void *)client_sock, 0, NULL);

	if (hthr != 0) {
		if (n_threads >= n_allocated) {
			n_allocated = n_threads + 10;
			h_threads = (HANDLE *)realloc(h_threads, n_allocated * sizeof (HANDLE));
		}

		h_threads[n_threads++] = hthr;
	}
}

void finish_threads(void)
{
	int i;

	WaitForMultipleObjects(n_threads, h_threads, TRUE, INFINITE);

	for (i = 0; i < n_threads; ++i)
		CloseHandle(h_threads[i]);

	free(h_threads);
}

int main()
{
	SOCKET accept;
	HANDLE h_exit;
	int err;

	if ((err = nw_init()) != 0) {
		fprintf(stderr, "Error initializing network (%d)\n", err);
		return 1;
	}

	if ((err = nw_listen(&accept, 44556)) != 0) {
		fprintf(stderr, "Error on listening (%d)\n", err);
		nw_shutdown();
		return 1;
	}

	h_exit = nw_install_break_handler();

	err = nw_accept_loop(accept, h_exit, accept_callback);

	fprintf(stderr, "nw_accept_loop() exited with %d\n", err);

	finish_threads();

	CloseHandle(h_exit);
	nw_closesocket(accept);
	nw_shutdown();

	return 0;
}
