#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <error.h>
#include <sys/types.h>
#include <pthread.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#define MAX_PAYLOAD 5000
struct sockaddr_in s_sockaddr, c_sockaddr;
socklen_t socklen;
pthread_t recv_tcpthread, recv_chrdevthread;

int res;
int d_ipport = 2551;
char d_ipaddr[16] = "127.0.0.1";
char file_name[32] = "tcptunl0";
int server = 0;

int chrdev_fd, tcpsock_fd, tcpsend_fd;

int readn(int fd, char *data, u_int16_t len);
void *read_from_tcpsock(void * nothing);
void *read_from_chrdev(void *nothing);
int writen(int fd, char *data, u_int16_t len);
void hexprint(char *data, u_int64_t len);

void *read_from_tcpsock(void * nothing)
{
	int res= 0;
	u_int16_t len = 0;
	char *data;
	data = malloc(MAX_PAYLOAD);
	if (data == NULL) {
		perror("malloc");
		pthread_exit(NULL);
	}
	while(1) {
		len = 0;
		res = readn(tcpsend_fd, (char*) &len, sizeof(u_int16_t));
		if(res < 0) {
			perror("read");
			pthread_exit(NULL);
		}
		//printf("len = %d",len);
		len = ntohs(len);
		printf("received %d bytes from tcpsock\n",len);
		if(len > MAX_PAYLOAD) {
			printf("length exceeds MAX_PAYLOAD");
			exit(EXIT_FAILURE);
		}
		res = readn(tcpsend_fd, data, len);
		if(res != len) {
			perror("readn");
			pthread_exit(NULL);
		}
		hexprint(data, len);
		res = write(chrdev_fd, data, len);
		if(res < 0 ) {
			perror("write in chrdev_fd in read_from_tcpsock");
			pthread_exit(NULL);
		}
	}
}

void *read_from_chrdev(void *nothing)
{
	int res = 0;
	u_int16_t len = 0;
	char *data;

	data = malloc(MAX_PAYLOAD);
	if( data ==NULL) {
		perror("malloc in readfromchrdev");
		pthread_exit(NULL);
	}
	while(1) {
		len = read(chrdev_fd, data, MAX_PAYLOAD);
		if(res < 0) {
			perror("read in readfromchrdev");
			pthread_exit(NULL);
		}
		len = htons(len);
		res = writen(tcpsend_fd, (char*) &len, sizeof(u_int16_t));
		if(res < 0) {
			perror("read in readfromchrdev");
			pthread_exit(NULL);
		}
		res = writen(tcpsend_fd, data, ntohs(len));
		if(res < 0) {
			perror("read in readfromchrdev");
			pthread_exit(NULL);
		}

	}

}

int readn(int fd, char *data, u_int16_t len)
{
	u_int16_t readlen = 0;
	int res;
	while(readlen < len) {
		//printf("len - readlen=%d\n",len-readlen);
		res = read(fd, (void *)(data + readlen), len - readlen);
		if(res == -1) {
			perror("in readn");
			return EXIT_FAILURE;
		}
		readlen = readlen + res;
	}
	return readlen;
}

int writen(int fd, char *data, u_int16_t len)
{
	u_int16_t writelen = 0;
	int res;
	while(writelen < len) {
		res = write(fd, (void *)(data + writelen), len - writelen);
		if(res == -1) {
			perror("writen");
			return EXIT_FAILURE;
		}
		writelen = writelen + res;
	}
	return writelen;
}

void hexprint(char *data, u_int64_t len)
{
	u_int64_t i;
	printf("\n------------------\n");
	for(i = 0; i < len; i++) {
		printf("%x",data[i]);
	}
	printf("\n------------------\n");

}

int main(int argc, char **argv)
{
	const char *short_opt = "d:p:sf:";
	const struct option long_option[] = {
		{"dest", 1, NULL, 'd'},
		{"port", 1, NULL, 'p'},
		{"serv", 0, NULL, 's'},
		{"file", 1, NULL, 'f'}
	};
	int next_arg;

	do{
		next_arg = getopt_long(argc,argv,short_opt,long_option,NULL);

		switch(next_arg)
		{
		case 'd':
			strcpy(d_ipaddr,optarg);
			break;
		case 'p':
			d_ipport = atoi(optarg);
			break;
		case 's':
			server = 1;
			break;
		case 'f':
			strcpy(file_name, optarg);
			break;
		case -1:
			break;
		case '?':
			break;
		}
	}while(next_arg != -1);

	chrdev_fd = open(file_name, O_RDWR);
	if(chrdev_fd == -1) {
		perror("open chrdev");
		exit(EXIT_FAILURE);
	}

	tcpsock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(tcpsock_fd < 0 ) {
		printf("error in socket\n");
		perror("socket tcpsock_fd");
		exit(EXIT_FAILURE);
	}
	bzero(&s_sockaddr, sizeof(s_sockaddr));
	bzero(&c_sockaddr, sizeof(c_sockaddr));

	s_sockaddr.sin_family = AF_INET;
	s_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	s_sockaddr.sin_port = htons(d_ipport);

	c_sockaddr.sin_family = AF_INET;
	c_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	c_sockaddr.sin_port = htons(INADDR_ANY);


	if(server) {

		res = bind(tcpsock_fd, (struct sockaddr *) &s_sockaddr, sizeof(s_sockaddr));
		if (res < 0) {
			printf("error in bind\n");
			perror("bind s_sockaddr");
			exit(EXIT_FAILURE);
		}
		res = listen(tcpsock_fd, 1024);
		if(res < 0) {
			perror("listen");
			exit(EXIT_FAILURE);
		}
		socklen = sizeof(c_sockaddr);
		tcpsend_fd = accept(tcpsock_fd, (struct sockaddr *)&c_sockaddr, &socklen);
		if(tcpsend_fd < 0) {
			printf("error in accept\n");
			perror("accept");
			exit(EXIT_FAILURE);
		}
	} else {
		c_sockaddr.sin_addr.s_addr = inet_addr(d_ipaddr);
		c_sockaddr.sin_port = htons(d_ipport);
		res = connect(tcpsock_fd, (struct sockaddr *)&c_sockaddr, sizeof(c_sockaddr));
		if(res < 0) {
			printf("error in connect\n");
			perror("connect:");
			exit(EXIT_FAILURE);
		}
		tcpsend_fd = tcpsock_fd;
	}
	pthread_create(&recv_tcpthread, NULL, read_from_tcpsock, NULL);
	pthread_create(&recv_chrdevthread, NULL, read_from_chrdev, NULL);
	pthread_join(recv_tcpthread, NULL);
	pthread_join(recv_chrdevthread, NULL);
	if(server)
		close(tcpsend_fd);
	close(tcpsock_fd);
	close(chrdev_fd);
	return (EXIT_SUCCESS);

}
