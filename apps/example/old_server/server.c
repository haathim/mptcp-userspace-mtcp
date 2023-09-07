#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <mtcp_api.h>

#define PORT_NUM 12345
#define MAX_CONNECTIONS 3

int main() {
    int ret;
    mctx_t mctx;

    // Initialize mTCP
    ret = mtcp_init("server.conf");
    if (ret) {
        fprintf(stderr, "mtcp_init failed: %s\n", strerror(-ret));
        return -1;
    }

    // Create mTCP context
    mctx = mtcp_create_context(0);
    if (!mctx) {
        fprintf(stderr, "mtcp_create_context failed\n");
        return -1;
    }

    // Create a socket
    int listen_sock = mtcp_socket(mctx, AF_INET, SOCK_STREAM, 0);
    if (listen_sock < 0) {
        fprintf(stderr, "mtcp_socket failed: %s\n", strerror(-listen_sock));
        return -1;
    }

    // Bind the socket
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT_NUM);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    ret = mtcp_bind(mctx, listen_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (ret) {
        fprintf(stderr, "mtcp_bind failed: %s\n", strerror(-ret));
        return -1;
    }

    // Listen for incoming connections
    ret = mtcp_listen(mctx, listen_sock, MAX_CONNECTIONS);
    if (ret) {
        fprintf(stderr, "mtcp_listen failed: %s\n", strerror(-ret));
        return -1;
    }

    printf("Server listening on port %d...\n", PORT_NUM);

    while (1) {
        // Accept a new client connection
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client_sock = mtcp_accept(mctx, listen_sock, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_sock < 0) {
            fprintf(stderr, "mtcp_accept failed: %s\n", strerror(-client_sock));
            //continue;
	    exit(1);
        }

        printf("Accepted a new connection from %s:%d\n",
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // Continuously receive and reply to client messages
        char buffer[1024];
        while (1) {
            ret = mtcp_recv(mctx, client_sock, buffer, sizeof(buffer) - 1, 0);
            if (ret <= 0) {
                fprintf(stderr, "mtcp_recv failed: %s\n", strerror(-ret));
                break;
            }
            buffer[ret] = '\0'; // Null-terminate received message
            printf("Received: %s\n", buffer);

            // Modify the received message and send it back to the client
            char reply[1024];
            snprintf(reply, sizeof(reply), "Server says: %s", buffer);

            ret = mtcp_write(mctx, client_sock, reply, strlen(reply));
            if (ret < 0) {
                perror("mtcp_write");
                return -1;
            }
        }

        // Close the client socket
        mtcp_close(mctx, client_sock);
        printf("Connection closed with %s:%d\n",
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    }

    // Close the listening socket and destroy the mTCP context
    mtcp_close(mctx, listen_sock);
    mtcp_destroy_context(mctx);

    return 0;
}



//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <arpa/inet.h>
//#include <mtcp_api.h>
//#include <mtcp_epoll.h>
//#include <signal.h>
//
//#define PORT_NUM 12345
//#define MAX_CONNECTIONS 4
//#define MAX_EVENTS 3
//
//// Signal handler function
////void handle_ctrl_c(int sig) {
////    printf("CTRL+C received. Exiting...\n");
////    exit(0);
////}
//
//int main() {
//
//    // Set up the signal handler for SIGINT (CTRL+C)
//    //if (signal(SIGINT, handle_ctrl_c) == SIG_ERR) {
//    //    perror("Error setting up signal handler");
//    //    return 1;
//    //}
//
//    int ret, ep, i, client_sock;
//    mctx_t mctx;
//    struct mtcp_epoll_event ev, events[MAX_EVENTS];
//
//    // Initialize mTCP
//    ret = mtcp_init("server.conf");
//    if (ret) {
//        fprintf(stderr, "mtcp_init failed: %s\n", strerror(-ret));
//        return -1;
//    }
//
//    // Create mTCP context
//    mctx = mtcp_create_context(0);
//    if (!mctx) {
//        fprintf(stderr, "mtcp_create_context failed\n");
//        return -1;
//    }
//
//    // Create a socket
//    int listen_sock = mtcp_socket(mctx, AF_INET, SOCK_STREAM, 0);
//    if (listen_sock < 0) {
//        fprintf(stderr, "mtcp_socket failed: %s\n", strerror(-listen_sock));
//        return -1;
//    }
//
//    // Bind the socket
//    struct sockaddr_in server_addr;
//    memset(&server_addr, 0, sizeof(server_addr));
//    server_addr.sin_family = AF_INET;
//    server_addr.sin_port = htons(PORT_NUM);
//    server_addr.sin_addr.s_addr = INADDR_ANY;
//
//    ret = mtcp_bind(mctx, listen_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
//    if (ret) {
//        fprintf(stderr, "mtcp_bind failed: %s\n", strerror(-ret));
//        return -1;
//    }
//
//    // Listen for incoming connections
//    ret = mtcp_listen(mctx, listen_sock, MAX_CONNECTIONS);
//    if (ret) {
//        fprintf(stderr, "mtcp_listen failed: %s\n", strerror(-ret));
//        return -1;
//    }
//
//    /* create epoll queue & enlist listening port in epoll queue */
//	ep = mtcp_epoll_create(mctx, MAX_EVENTS);
//	ev.events = MTCP_EPOLLIN; ev.data.sockid = listen_sock;
//	mtcp_epoll_ctl(mctx, ep, MTCP_EPOLL_CTL_ADD, listen_sock, &ev);
//
//    printf("Server listening on port %d...\n", PORT_NUM);
//
//    char buffer[2048];
//
//    while (1) {
//
//	int nevents = mtcp_epoll_wait(mctx, ep, events, MAX_EVENTS, -1);
//	printf("No of events: %d\n", nevents);
//	for (i = 0; i < nevents; i++) {
//		if (events[i].data.sockid == listen_sock) {
//	            client_sock = mtcp_accept(mctx, listen_sock, NULL, NULL);
//		    //ret = mtcp_recv(mctx, client_sock, buffer, sizeof(buffer) - 1, 0);
//		    ret = mtcp_read(mctx, client_sock, buffer, sizeof(buffer) - 1);
//		    if (ret <= 0) {
//			fprintf(stderr, "mtcp_recv failed: %s\n", strerror(-ret));
//			break;
//		    }
//		    buffer[ret] = '\0';  // Null-terminate received message
//		    printf("Received: %s\n", buffer);
//
//		    // Modify the received message and send it back to the client
//		    char reply[1024];
//		    snprintf(reply, sizeof(reply), "Server says: %s", buffer);
//
//			ret = mtcp_write(mctx, client_sock, reply, strlen(reply));
//			if (ret < 0) {
//				perror("mtcp_write");
//				return -1;
//			}
//		} else if (events[i].events == MTCP_EPOLLIN) {
//			printf("IT'S A EPOLLIN EVENT!\n");
//		}
//
//	}
//	 
//        // Accept a new client connection
//        //struct sockaddr_in client_addr;
//        //socklen_t client_addr_len = sizeof(client_addr);
//        //int client_sock = mtcp_accept(mctx, listen_sock, (struct sockaddr*)&client_addr, &client_addr_len);
//        //int client_sock = mtcp_accept(mctx, listen_sock, NULL, NULL);
//        //if (client_sock < 0) {
//        //    fprintf(stderr, "mtcp_accept failed: %s\n", strerror(-client_sock));
//        //    continue;
//        //}
//
//        //printf("Accepted a new connection from %s:%d\n",
//         //      inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
//
//        // Continuously receive and reply to client messages
//        //char buffer[1024];
//        //while (1) {
//        //    ret = mtcp_recv(mctx, client_sock, buffer, sizeof(buffer) - 1, 0);
//        //    if (ret <= 0) {
//        //        fprintf(stderr, "mtcp_recv failed: %s\n", strerror(-ret));
//        //        break;
//        //    }
//        //    buffer[ret] = '\0';  // Null-terminate received message
//        //    printf("Received: %s\n", buffer);
//
//        //    // Modify the received message and send it back to the client
//        //    char reply[1024];
//        //    snprintf(reply, sizeof(reply), "Server says: %s", buffer);
//
//	//        ret = mtcp_write(mctx, client_sock, reply, strlen(reply));
//	//	if (ret < 0) {
//	//		perror("mtcp_write");
//	//		return -1;
//	//	}
//        //}
//
//        // Close the client socket
//        //mtcp_close(mctx, client_sock);
//        //printf("Connection closed with %s:%d\n",
//         //      inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
//    }
//
//    // Close the listening socket and destroy the mTCP context
//    mtcp_close(mctx, listen_sock);
//    mtcp_destroy_context(mctx);
//
//    return 0;
//}
//
