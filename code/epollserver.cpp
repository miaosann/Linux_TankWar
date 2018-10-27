#include <iostream>
#include <list>

#include  <unistd.h>
#include  <sys/types.h>       /* basic system data types */
#include  <sys/socket.h>      /* basic socket definitions */
#include  <netinet/in.h>      /* sockaddr_in{} and other Internet defns */
#include  <arpa/inet.h>       /* inet(3) functions */
#include <sys/epoll.h> /* epoll function */
#include <fcntl.h>     /* nonblocking */
#include <sys/resource.h> /*setrlimit */

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>


#define MAXEPOLLSIZE 10000
#define MAXLINE 1024
using namespace std;
int handle(int connfd,vector<int> connfd_vector);
int setnonblocking(int sockfd)
{
	if (fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0)|O_NONBLOCK) == -1) {////设置成非阻塞模式； 
		return -1;
	}
	return 0;
}

int main(int argc, char **argv)
{
	int  servPort = 8080;
	int listenq = 1024;

	int listenfd, connfd, kdpfd, nfds, n, nread, curfds,acceptCount = 0;
	struct sockaddr_in servaddr, cliaddr;
	socklen_t socklen = sizeof(struct sockaddr_in);
	struct epoll_event ev;
	struct epoll_event events[MAXEPOLLSIZE];
	struct rlimit rt;	//进程资源的限制
	char buf[MAXLINE];

	vector<int> connfd_vector;
	vector<int>::iterator viter;
	list<int> connfd_list;
    list<int>::iterator iter;
	/* 设置每个进程允许打开的最大文件数 */
	/**
		* rlimit {
		　　rlim_t rlim_cur;　　//soft limit
		　　rlim_t rlim_max;　　//hard limit
		};
	 * */
	rt.rlim_max = rt.rlim_cur = MAXEPOLLSIZE; 
	if (setrlimit(RLIMIT_NOFILE, &rt) == -1) 
	{
		perror("setrlimit error");
		return -1;
	}


	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl (INADDR_ANY);
	servaddr.sin_port = htons (servPort);

	listenfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (listenfd == -1) {
		perror("can't create socket file");
		return -1;
	}

	int opt = 1;
	//在TCP连接中，recv等函数默认为阻塞模式(block)，即直到有数据到来之前函数不会返回，
	//而我们有时则需要一种超时机制使其在一定时间后返回而不管是否有数据到来，这里我们就会用到setsockopt()函数
	/**
	 * int setsockopt(int sock, int level, int optname, const void *optval, socklen_t optlen);
	 * sock：将要被设置或者获取选项的套接字。
		level：选项所在的协议层。
		optname：需要访问的选项名。
		optval：对于getsockopt()，指向返回选项值的缓冲。对于setsockopt()，指向包含新选项值的缓冲。
		optlen：对于getsockopt()，作为入口参数时，选项值的最大长度。作为出口参数时，选项值的实际长度。对于setsockopt()，现选项的长度
	 * */
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	if (setnonblocking(listenfd) < 0) {
		perror("setnonblock error");
	}

	if (bind(listenfd, (struct sockaddr *) &servaddr, sizeof(struct sockaddr)) == -1) 
	{
		perror("bind error");
		return -1;
	} 
	if (listen(listenfd, listenq) == -1) 	//同时监听1024个
	{
		perror("listen error");
		return -1;
	}
	/* 创建 epoll 句柄，把监听 socket 加入到 epoll 集合里 */
	kdpfd = epoll_create(MAXEPOLLSIZE);
	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = listenfd;
	if (epoll_ctl(kdpfd, EPOLL_CTL_ADD, listenfd, &ev) < 0) 
	{
		fprintf(stderr, "epoll set insertion error: fd=%d\n", listenfd);
		return -1;
	}
	curfds = 1;

	printf("epollserver startup,port %d, max connection is %d, backlog is %d\n", servPort, MAXEPOLLSIZE, listenq);

	for (;;) {
		/* 等待有事件发生 */
		nfds = epoll_wait(kdpfd, events, curfds, -1);
		//epoll_wait返回之后应该是一个循环，遍历所有的事件。
		if (nfds == -1)
		{
			perror("epoll_wait");
			continue;
		}
		/* 处理所有事件 */
		for (n = 0; n < nfds; ++n)
		{
			if (events[n].data.fd == listenfd) 
			{
				connfd = accept(listenfd, (struct sockaddr *)&cliaddr,&socklen);
				
                connfd_list.push_back(connfd);
				connfd_vector.push_back(connfd);
                if (connfd < 0) 
				{
					perror("accept error");
					continue;
				}

				sprintf(buf, "accept form %s:%d\n", inet_ntoa(cliaddr.sin_addr), cliaddr.sin_port);
				printf("%d:%s", ++acceptCount, buf);
			
				if (curfds >= MAXEPOLLSIZE) {
					fprintf(stderr, "too many connection, more than %d\n", MAXEPOLLSIZE);
					close(connfd);
					continue;
				} 
				if (setnonblocking(connfd) < 0) {
					perror("setnonblocking error");
				}
				ev.events = EPOLLIN | EPOLLET;
				ev.data.fd = connfd;
				if (epoll_ctl(kdpfd, EPOLL_CTL_ADD, connfd, &ev) < 0)
				{
					fprintf(stderr, "add socket '%d' to epoll failed: %s\n", connfd, strerror(errno));
					return -1;
				}
			/*
				int haha= 0;
				for(haha=0;haha<curfds;haha++){
					
					//printf("have connected clients include  %s:%d\n", inet_ntoa(cliaddr.sin_addr), cliaddr.sin_port);
					printf("we have socket-return fd include : %d\n",events[haha].data.fd);
				}
			*/
				curfds++;
				continue;
			} 
			
			// 处理客户端请求
			if (handle(events[n].data.fd,connfd_vector) < 0) {
                for(iter=connfd_list.begin();iter!=connfd_list.end();){
                    if(*iter==events[n].data.fd)
                        iter = connfd_list.erase(iter);
                    else
                        iter++;
                }
				for(viter=connfd_vector.begin();viter!=connfd_vector.end();){
                    if(*viter==events[n].data.fd)
                        viter = connfd_vector.erase(viter);
                    else
                        viter++;
                }
				epoll_ctl(kdpfd, EPOLL_CTL_DEL, events[n].data.fd,&ev);
				curfds--;
				close(connfd);
			}
		}
	}
	close(kdpfd);
	close(listenfd);
	return 0;
}
int handle(int connfd,vector<int> connfd_vector){
	int nread;
	char buf[MAXLINE];
	nread = read(connfd, buf, MAXLINE);//读取客户端socket流
	


	if (nread == 0) {
		printf("client close the connection\n");
		close(connfd);
		return -1;
	} 
	if (nread < 0) {
		perror("read error");
		close(connfd);
		return -1;
	}	
	int index;//记录正在执行的是第几个connfd
	//write(connfd, buf, nread);//响应客户端  
	list<int>::iterator iter;
	// for(iter=connfd_list.begin();iter!=connfd_list.end();iter++){
    //     int conn = *iter;
	// 	if(conn!=connfd){
	// 		write(conn, buf, nread);
	// 		fflush;
	// 		//printf("%d:  %s",conn,buf);
	// 	}
	// }
	for(int i = 0 ; i < connfd_vector.size();i++){
		//首先找到该connfd是第几个
		if(connfd == connfd_vector[i]){
			index = i;
			break;
		}else{
			continue;
		}
	}
	if(index%2==0&&(index+1)<connfd_vector.size()){//每组的第一个,且不是最后一组
		int conn = connfd_vector[index+1];
		write(conn, buf, nread);
		fflush;
	}
	else if(index%2==1){//每组第二个
		int conn = connfd_vector[index-1];
		write(conn, buf, nread);
		fflush;
	}
	
	//printf("%s",buf);
	memset(buf,0,sizeof(buf));
	fflush;
	//write(connfd, "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n<html> <body> Hello client!  </body> </html>", strlen("HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n<html> <body> Hello client!  </body> </html>"));
	//close(connfd);
	return 0;
}
