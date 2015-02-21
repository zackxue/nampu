
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ev.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>

#include "rtsp_msg.h"
#include "rtsp_parse.h"
#include "rtsp_url.h"
#include "rtsp_transport.h"
#include "rtsp_sdp.h"
#include "rtsp_log.h"


static int test_num;

#define print_line() \
	print("\n[%d] ********************* [ %d ] ********************* \n\n", \
	__LINE__, ++test_num);


#define PORT		8003	//监听端口
#define ADDR_IP	"192.168.1.12"	//IP

#define RECV_BUFFER_SIZE	(4096)


int socket_init();
void accept_callback(struct ev_loop *loop, ev_io *w, int revents);
void recv_callback(struct ev_loop *loop, ev_io *w, int revents);
void write_callback(struct ev_loop *loop, ev_io *w, int revents);

void rtsp_test();
void sdp_test();
void rtsp_url_test();
void rtsp_transport_test();


/*
 *	传入str指针所指内存长度必须不小于11
 */
void my_itoa(uint32 num, char *str)
{
	int i = 0;
	int len = 0;
	uint32 num_test = num;

	func_begin("\n");

	if (num_test == 0)
	{
		str[0] = '0';
		str[1] = '\0';
		return ;
	}

	while (num_test > 0)
	{
		len++;
		num_test /= 10;
	}

	num_test = num;
	for (i = 0; i < len; i++)
	{
		str[len - 1 - i] = num_test % 10 + '0';
		num_test /= 10;
	}
	str[len] = '\0';
}


/*
 *	客户端输入:	nc 192.168.1.12 8003
 */
int main(int argc, char **argv)
{
	//rtsp_test();
	//sdp_test();
	//rtsp_url_test();
	rtsp_transport_test();

	int listen;
	ev_io ev_io_watcher;

	listen = socket_init();
	struct ev_loop *loop = ev_loop_new(EVBACKEND_EPOLL);
	ev_io_init(&ev_io_watcher, accept_callback, listen, EV_READ);
	ev_io_start(loop, &ev_io_watcher);
	ev_run(loop, 0);
	ev_loop_destroy(loop);

	return 0;
}


int socket_init()
{
	struct sockaddr_in my_addr;
	int listener;
	
	if ((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	} else {
		log_1("socket create success\n");
	}

	int so_reuseaddr=1;
	setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &so_reuseaddr, 
		sizeof(so_reuseaddr));
	bzero(&my_addr, sizeof(my_addr));
	my_addr.sin_family = PF_INET;
	my_addr.sin_port = htons(PORT);
	my_addr.sin_addr.s_addr = inet_addr(ADDR_IP);

	if (bind(listener, (struct sockaddr *) &my_addr, sizeof(struct sockaddr))== -1) {
		perror("bind error!\n");
		exit(1);
	} else {
		log_1("IP BIND SUCCESS, IP:%s\n", ADDR_IP);
	}

	if (listen(listener, 1024) == -1) {
		perror("listen error!\n");
		exit(1);
	} else {
		log_1("LISTEN SUCCESS, PORT:%d\n", PORT);
	}
	
	return listener;
}


void accept_callback(struct ev_loop *loop, ev_io *w, int revents)
{
	int newfd;
	struct sockaddr_in sin;
	socklen_t addrlen = sizeof(struct sockaddr);
	ev_io *accept_watcher = my_alloc(sizeof(ev_io));

	log_1("accept_callback begin...\n");
	while ((newfd = accept(w->fd, (struct sockaddr *)&sin, &addrlen)) < 0)
	{
		int cur_error = errno;
		if (cur_error == EAGAIN || cur_error == EWOULDBLOCK) {
			continue;
		} else {
			log_0("accept error.[%s]\n", strerror(cur_error));
			break;
		}
	}
	
	ev_io_init(accept_watcher, recv_callback, newfd, EV_READ);
	ev_io_start(loop, accept_watcher);
	log_1("accept callback: fd: %d\n", accept_watcher->fd);
}


void recv_callback(struct ev_loop *loop, ev_io *w, int revents)
{
	rtsp_parser *parser;
	rtsp_message *rtsp_msg;
	RTSP_RESULT res;
	int fd;
	int err;

	parser = rtsp_parser_new(RECV_BUFFER_SIZE);
	res = rtsp_parser_recv(parser, w->fd, &rtsp_msg, &err);
	log_1("rtsp_ctx_recv res = %d, err = %s\n", res, strerror(err));
	if (res != RTSP_OK) {
		if (res == RTSP_EEOF)
			log_1("rtsp_parser_recv return RTSP_EEOF\n");
		else
			log_0("rtsp_parser_recv, error, res = %d\n", res);
		goto recv_err;
	}
	rtsp_message_dump(rtsp_msg);

	char rtsp_to_string[791];
	int string_len = 0;
	string_len = rtsp_message_to_string(rtsp_msg, rtsp_to_string, sizeof(rtsp_to_string));
	if (string_len < 0 || string_len >= sizeof(rtsp_to_string)) {
		log_0("string_len(%d) < 0 || string_len >= sizeof(rtsp_to_string)(%u), " \
		"not complete copy\n", string_len, sizeof(rtsp_to_string));
		goto to_string_err;
	}
	log_2("rtsp_message_to_string, result is:\n%s\n", rtsp_to_string);
	
/* sdp测试 */
	sdp_message *sdp_msg;

	sdp_message_new(&sdp_msg);
	sdp_message_parse_buffer(rtsp_msg->body, rtsp_msg->body_size, sdp_msg);
	sdp_message_dump(sdp_msg);

	char str[555];
	int str_len;
	str_len = sdp_message_as_text(sdp_msg, str, sizeof(str));
	if (str_len >= sizeof(str))
		log_0("str_len(%u) >= sizeof(str)(%u), not complete copy\n", 
		str_len, sizeof(str));
	log_2("sdp_message_as_text, result is:\n%s\n", str);
	
	sdp_message_free(sdp_msg);
/* sdp测试  end */

recv_err:
to_string_err:
	rtsp_parse_free(parser);
	
	fd = w->fd;
	ev_io_stop(loop, w);	//把原来的读事件停止
	ev_io_init(w, write_callback, fd, EV_WRITE);	//重新初始化事件为写事件
	ev_io_start(loop, w);
}


void write_callback(struct ev_loop *loop, ev_io *w, int revents)
{
	char buffer[1024] = {0};
	snprintf(buffer, 1023, "-----happy-----\n");
	write(w->fd, buffer, strlen(buffer));

	int fd = w->fd;
	ev_io_stop(loop, w);	//把写事件停止
	ev_io_init(w, recv_callback, fd, EV_READ);	//初始化为读事件
	ev_io_start(loop, w);
}


void rtsp_transport_test()
{
	char ch[2][200] = {
		{"RTP/AVP;unicast;client_port=15016-15017"},
		{"RTP/AVP;unicast;client_port=15016-15017;server_port=57496-57497;ssrc=6043D8FD;mode=\"PLAY\""}
	};
	int i;
	rtsp_transport *transport;

	if (rtsp_transport_new(&transport) != RTSP_OK)
	{
		log_0("rtsp_transport_new error\n");
		return ;
	}
	
	for (i = 0; i < 2; i++)
	{
		log_2("ch[%d]:%s\n", i, ch[i]);
		//rtsp_transport_init(transport);
		rtsp_transport_parse(ch[i], transport);

		char str_text[1024] = {0};
		int str_len;
		
		str_len = rtsp_transport_as_text(transport, str_text, sizeof(str_text));
		log_2("str_len = %d\n", str_len);
		if (str_len > 0)
			log_2("str_text:%s\n", str_text);
	}

	rtsp_transport_free(transport);
}


void rtsp_url_test()
{
	rtsp_url *url;
	RTSP_RESULT res;
	char ch[200] = "rtsp[u]://user:passwd@host:12233/abspath?happy_query";
	log_2("ch:%s\n", ch);

	res = rtsp_url_parse(ch, &url);
	log_2("rtsp_url_parse, res = %d\n", res);
	
	if (res == RTSP_OK) {
		log_2("user:%s\n", url->user.str);
		log_2("passwd:%s\n", url->passwd.str);
		log_2("host:%s\n", url->host.str);
		log_2("port:%d\n", url->port);
		log_2("abspath:%s\n", url->abspath.str);
		log_2("query:%s\n", url->query.str);
		print("\n");
	}

	char *uri;
	uri = rtsp_url_get_request_uri(url);
	log_2("uri:%s\n", uri);
	print("\n");
	my_free(uri, STRLEN1(uri));

	rtsp_url_free(url);
}


void sdp_test()
{
/* sdp测试 */
	sdp_message *sdp_msg;
	char sdp_uri_test[1024] = "http://www.google.com.hk/search?hl=zh-CN&newwindow=1" \
		"&safe=strict&site=&source=hp&q=GstSDPMedia&btnK=Google+%E6%90%9C%E7%B4%A2";

	sdp_message_new(&sdp_msg);
	sdp_message_parse_uri(sdp_uri_test, sdp_msg);
	sdp_message_dump(sdp_msg);

	char str[4096];
	sdp_message_as_text(sdp_msg, str, 4096);
	log_2("sdp_message_as_text, result is:\n%s\n", str);
	
	sdp_message_free(sdp_msg);
/* sdp测试  end */
}


#if 1
void rtsp_test()
{
	printf("\nrtsp-zyt test begin...\n");

	/* 测试:	my_itoa */
	print_line();
	{
		uint32 a = 0;
		char str[12];
		my_itoa(a, str);
		log_1("str = %s\n", str);

		a = 2012356789;	//取4012356789编译会有警告，但不影响
		my_itoa(a, str);
		log_1("str = %s\n", str);
	}
#if 0
	/* 测试:	rtsp_key_values_new */
	print_line();
	{
		rtsp_key_values *key_values = NULL;
		log_1("key_values addr: %p\n", key_values);
		rtsp_key_values_new(&key_values);
		log_1("key_values addr: %p\n", key_values);
		log_1("key_values->size = %d\n", key_values->size);

		rtsp_key_values_new(&key_values);
		my_free(key_values, sizeof(rtsp_key_values));
	}
#endif
	/* 测试:	rtsp_message_new */
	print_line();
	{
		rtsp_message *msg = NULL;
		if (rtsp_message_new(&msg) == RTSP_OK)
			log_1("rtsp_message_new success, msg addr = %p\n", msg);
		else
			log_1("rtsp_message_new failed\n");

		RTSP_MSG_TYPE msg_type = rtsp_message_get_type(msg);
		if (msg_type == RTSP_MESSAGE_INVALID)
			log_1("msg_type = RTSP_MESSAGE_INVALID\n");
		else
			log_1("msg_type != RTSP_MESSAGE_INVALID\n");

		if (rtsp_message_free(msg) == RTSP_OK)
			log_1("rtsp_message_free success\n");
		else
			log_1("rtsp_message_free failed\n");
	}

	/* 测试:	rtsp_message_new_request */
	print_line();
	{
		rtsp_message *msg = NULL;
		if (rtsp_message_new_request(&msg, RTSP_GET, "www.hao123.com") == RTSP_OK)
			log_1("rtsp_message_new_request success, msg addr = %p\n", msg);
		else
			log_1("rtsp_message_new_request failed\n");

		RTSP_METHOD cur_method;
		const char *uri;
		uint32 uri_len;
		RTSP_VERSION cur_version;
		
		rtsp_message_parse_request(msg, &cur_method, &uri, &uri_len, &cur_version);
		log_1("cur_method is %s\n", rtsp_method_as_text(cur_method));
		log_1("uri is %s\n", uri);
		log_1("uri_len = %d\n", uri_len);
		log_1("cur_version is %s\n", rtsp_version_as_text(cur_version));

		{
			char str_out[1024];
			uint32 out_len = 0;
			int i;
			
			out_len = rtsp_message_to_string(msg, str_out, sizeof(str_out));
			log_1("out_len = %d\n", out_len);
			log_1("str_out is :\n");
			for (i = 0; i < out_len; i++)
				print("%c", str_out[i]);
			print("\n");
		}
		{
			print_line();
			rtsp_message_dump(msg);
		}

		{
			rtsp_message *msg_dup;
			print_line();
			msg_dup = rtsp_message_dup_request(msg);
			if (msg_dup != NULL)
			{	
				print_line();
				rtsp_message_dump(msg_dup);
				rtsp_message_free(msg_dup);
			}
			else
				log_1("msg_dup == NULL\n");
		}

		{
			print_line();
			rtsp_message *msg_response;
			if (rtsp_message_new_response(&msg_response, RTSP_STS_OK, "reason_123456", 
				msg) == RTSP_OK)
				log_1("rtsp_message_new_response success, msg addr = %p\n", msg);
			else
				log_1("rtsp_message_new_response failed\n");

			RTSP_STATUS_CODE cur_code;
			const char *reason;
			uint32 reason_len;
			RTSP_VERSION cur_version_response;
			
			rtsp_message_parse_response(msg_response, &cur_code, &reason, 
				&reason_len, &cur_version_response);
			if (cur_code == RTSP_STS_OK) log_1("cur_code is RTSP_STS_OK\n");
			log_1("reason is %s\n", reason);
			log_1("reason_len = %d\n", reason_len);
			log_1("cur_version_response is %s\n", 
				rtsp_version_as_text(cur_version_response));

			{
				print_line();
				rtsp_message_dump(msg_response);
			}

			if (rtsp_message_free(msg_response) == RTSP_OK)
				log_1("rtsp_message_free success\n");
			else
				log_1("rtsp_message_free failed\n");
		}
		
		if (rtsp_message_free(msg) == RTSP_OK)
			log_1("rtsp_message_free success\n");
		else
			log_1("rtsp_message_free failed\n");
	}

	/* 测试:	rtsp_message_new_data */
	print_line();
	{
		rtsp_message *msg = NULL;
		uint8 kaka[100] = "123456789abcdefghikdkdjflakjldjfi" \
				"djidjifajldjfldlfiajdiidfadf";
		/*
		if (rtsp_message_new_data(&msg, 64) == RTSP_OK)
			log_1("rtsp_message_new_data success, msg addr = %p\n", msg);
		else
			log_1("rtsp_message_new_data failed\n");

		log_1("channel = %d\n", msg->type_data.data.channel);
		uint8 cur_channel;
		rtsp_message_parse_data(msg, &cur_channel);
		log_1("cur_channel = %d\n", cur_channel);
		*/
		
		if (rtsp_message_new_request(&msg, RTSP_GET, "www.hao123.com") == RTSP_OK)
			log_1("rtsp_message_new_request success, msg addr = %p\n", msg);
		else
			log_1("rtsp_message_new_request failed\n");
		
		{
			if (rtsp_message_set_body(msg, kaka, 51) == RTSP_OK)
				log_1("rtsp_message_set_body success\n");
			else
				log_1("rtsp_message_set_body failed\n");

			
			{
				print_line();
				char str_out[150];
				uint32 out_len = 0;
				int i;
				
				out_len = rtsp_message_to_string(msg, str_out, sizeof(str_out));
				log_1("out_len = %d\n", out_len);
				log_1("str_out is :\n");
				for (i = 0; i < out_len; i++)
					print("%c", str_out[i]);
				print("\n");
			}
			
			uint8 *body;
			uint32 body_size;
			int i;

			rtsp_message_get_body(msg, &body, &body_size);
			log_1("body is:\n");
			for (i = 0; i < body_size; i++)
				print("%c", body[i]);
			print("\n");
			print_line();
			rtsp_message_dump(msg);

			
			rtsp_message_steal_body(msg, &body, &body_size);
			log_1("");
			for (i = 0; i < body_size; i++)
				print("%c", body[i]);
			print("\n");
			print_line();
			rtsp_message_dump(msg);

			rtsp_message *msg_dup;
			print_line();
			msg_dup = rtsp_message_dup_request(msg);
			if (msg_dup != NULL)
			{	
				print_line();
				rtsp_message_dump(msg_dup);
				rtsp_message_free(msg_dup);
			}
			else
				log_1("msg_dup == NULL\n");

		}

		if (rtsp_message_free(msg) == RTSP_OK)
			log_1("rtsp_message_free success\n");
		else
			log_1("rtsp_message_free failed\n");
	}

	/* 测试:	my_itoa */
	print_line();
	{
		
	}


	print_line();
}
#endif

