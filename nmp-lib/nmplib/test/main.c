#include <unistd.h>
#include <stdio.h>
#include <nmplib.h>

nmp_threadpool_t *tp;


void th_func1(void *data)
{       
	printf("pthread_pool==> data : %p\n", data);
}

void th_func(void *data, void *user_data)
{
    pthread_t thread_id = 0;

    printf("pthread_pool==> id = %u\n", thread_id);
    nmp_thread_self(&thread_id);

	printf("pthread_pool==> data : %p, user_data : %p\n", data, user_data);
	printf("pthread_pool==> id = %u\n", thread_id);

    sleep(3);
}


void tp_test( void )
{
	tp = nmp_threadpool_new(th_func, (void*)0xff, 4, NULL);
	sleep(1);
	nmp_threadpool_push(tp, (void*)1);
	sleep(1);
	nmp_threadpool_push(tp, (void*)2);
	sleep(1);
	nmp_threadpool_push(tp, (void*)3);
	sleep(1);
}

int main(int argc, char *argv[])
{
    pthread_t t;

	printf("\n\n******** Date: %s %s *********\n\n\n", __DATE__, __TIME__);
    
    pthread_create(&t, NULL, th_func1, 10);

	tp_test();

    pthread_join(t, NULL);
	return 0;
}
