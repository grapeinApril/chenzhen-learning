int main(int argc, char *argv[]) {

	unsigned short port = 9999;
	int sockfd = init_server(port);

	struct io_uring_params params;
	memset(&params, 0, sizeof(params));

	struct io_uring ring;
	io_uring_queue_init_params(ENTRIES_LENGTH, &ring, &params);
	// 初始化submission queue 、 completed queue
	// 该函数调用了系统调用API ： io_uring_setup
	
#if 0
	struct sockaddr_in clientaddr;	
	socklen_t len = sizeof(clientaddr);
	accept(sockfd, (struct sockaddr*)&clientaddr, &len);
#else

	struct sockaddr_in clientaddr;	
	socklen_t len = sizeof(clientaddr);
	set_event_accept(&ring, sockfd, (struct sockaddr*)&clientaddr, &len, 0);
	
#endif

	char buffer[BUFFER_LENGTH] = {0};

	while (1) {

		io_uring_submit(&ring);
		// 调用系统调用API： io_uring_enter

		struct io_uring_cqe *cqe;
		io_uring_wait_cqe(&ring, &cqe);
		// 取competed queue 的开始位置

		struct io_uring_cqe *cqes[128];
		int nready = io_uring_peek_batch_cqe(&ring, cqes, 128);  // epoll_wait
		// 从开始位置带出 至多【128】的元素

		int i = 0;
		for (i = 0;i < nready;i ++) {

			struct io_uring_cqe *entries = cqes[i];
			struct conn_info result;
			memcpy(&result, &entries->user_data, sizeof(struct conn_info));

			if (result.event == EVENT_ACCEPT) {

				set_event_accept(&ring, sockfd, (struct sockaddr*)&clientaddr, &len, 0);
				// 在该函数中调用了io_uring_prep_accept，其中与accept不同的点在多了第一个参数&ring
				// 提交请求到sqe里面
				//printf("set_event_accept\n"); //

				int connfd = entries->res;

				set_event_recv(&ring, connfd, buffer, BUFFER_LENGTH, 0);
				// 在该函数中调用了 io_uring_prep_recv，其中与recv不同的点在多了第一个参数&ring
				// 
				
			} else if (result.event == EVENT_READ) {  //

				int ret = entries->res;
				//printf("set_event_recv ret: %d, %s\n", ret, buffer); //

				if (ret == 0) {
					close(result.fd);
				} else if (ret > 0) {
					set_event_send(&ring, result.fd, buffer, ret, 0);
				}
			}  else if (result.event == EVENT_WRITE) {
  //

				int ret = entries->res;
				//printf("set_event_send ret: %d, %s\n", ret, buffer);

				set_event_recv(&ring, result.fd, buffer, BUFFER_LENGTH, 0);
			}
		}

		io_uring_cq_advance(&ring, nready);
		// 清空已经处理过的cq
	}

}
