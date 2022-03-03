#ifndef CLIENT_HPP
#define CLIENT_HPP
void sendPacket(int sockfd, unsigned char *packet);
void *run_client_thread(void *var);
#endif