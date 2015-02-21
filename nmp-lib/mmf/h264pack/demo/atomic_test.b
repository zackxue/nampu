#include <stdio.h>  
#include <pthread.h>  
#include <stdlib.h>
static atomic_t count = {0};
//static int count = 0;  
  
void *test_func(void *arg)  
{  
		int i=0;  
        for(i=0;i<20000000;++i){  
			atomic_inc(&count);
			atomic_dec(&count);
		//__sync_fetch_and_add(&count,1);  
		//__sync_fetch_and_sub(&count,1);  
		}  
		return NULL;  
}  
  
int main(int argc, const char *argv[])  
{  
		pthread_t id[20];  
		int i = 0;  
				  
		for(i=0;i<20;++i){  
			pthread_create(&id[i],NULL,test_func,NULL);  
		}  
				  
		for(i=0;i<20;++i){  
			pthread_join(id[i],NULL);  
		}  
					  
		printf("%d, _gnuc_:%d\n",count, __GNUC__);  
		return 0;  
}  
