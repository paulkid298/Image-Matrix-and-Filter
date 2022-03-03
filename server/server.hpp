#ifndef SERVER_HPP
#define SERVER_HPP
void *run_server_thread(void *var);
void receivePacket(unsigned char *packet, int connfd);
#endif