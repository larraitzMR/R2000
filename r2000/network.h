
#ifndef NETWORK_H_

#define NETWORK_H_


#define IP_ADDRESS		"192.168.1.52"


int configure_udp_socket(int port);
int send_udp_msg(int socket_fd, char *data, int port);
int read_udp_message(int sock_descriptor, char *message, char len);
int configure_tcp_socket(int port);
int send_tcp_msg(int socket_fd, char *data, int port);
int read_tcp_message(int sock_descriptor, char *message, char len);


#endif /* NETWORK_H_ */
