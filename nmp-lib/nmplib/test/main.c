
#include <stdio.h>
#include <nmplib.h>

JThreadPool *tp;


void *th_func1(void *data)
{
	printf("pthread_pool==> data : %p\n", data);
}
void th_func(void *data, void *user_data)
{
	printf("pthread_pool==> data : %p, user_data : %p\n", data, user_data);
}


void tp_test( void )
{
	tp = j_thread_pool_new(th_func, (void*)0xff, 1, NULL);
	sleep(1);
	j_thread_pool_push(tp, (void*)1);
	sleep(1);
	j_thread_pool_push(tp, (void*)2);
	sleep(1);
	j_thread_pool_push(tp, (void*)3);
	sleep(1);
}

int main(int argc, char *argv[])
{
	printf("\n\n******** Date: %s %s *********\n\n\n", __DATE__, __TIME__);
	pthread_t t;
        pthread_create(&t, NULL, th_func1, 10);

	tp_test();
	return 0;
}
