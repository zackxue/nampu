#include <stdio.h>
#include "mb_api.h"


static pu_process_cb_t get_cb;
static int get_process(pu_process_cb_t *hdle, mb_pu_parm_t *parm)
{
	return 0;
}


int main()
{
    get_cb.user_arg = NULL;
    get_cb.callback = get_process;
    if(pu_mb_init("224.0.1.2", 40086, &get_cb) != 0)
    {
        printf("pu_init_failed!\n");
        return -1;
    }
	
	return 0;
}
