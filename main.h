#ifndef MAIN_H
#define MAIN_H

extern struct net_device *tcptun_netdev;
extern int sock_type; /* module is server or client*/
extern int sock_port; /*tcp port*/
extern struct socket *sock, *client;
#endif // MAIN_H
