#include "nmp_mod_bss.h"
#include "nmp_ports.h"
#include "nmp_share_errno.h"
#include "nmp_bss_struct.h"
#include "message/nmp_msg_bss.h"
#include "nmp_share_debug.h"
#include "nmp_message.h"
#include "nmp_memory.h"
#include "search_device.h"


search_pu_list *g_search_pu_list = NULL;


void nmp_free_search_pu_list()
{
	gint size;

	if (g_search_pu_list)
	{
		size = g_search_pu_list->count*sizeof(search_result_t);
		nmp_mem_kfree(g_search_pu_list, size);
		g_search_pu_list = NULL;
	}
}
/*
void
nmp_set_search_pu_list(search_array_t *pu_list)
{
	gint size, count = 10,i;
	search_pu_list *search_pu = NULL;
      search_result_t  test[10];

      for (i = 0; i <10; i++)
      {
      		memset(&test[i], 0, sizeof(search_result_t));
		sprintf(test[i].dst_id, "test:11:22:33:44:55:%2d",i);
		test[i].nmp_srch.dev_info.av_mun = 8;
		test[i].nmp_srch.dev_info.pu_type = 2;
		sprintf(test[i].nmp_srch.dev_info.dev_ip, "192.168.1.%d",i);
		strcpy(test[i].nmp_srch.dev_info.mnfct,"NMP");
      }

	size = sizeof(NmpSearchPuRes) + count*sizeof(search_result_t);
	search_pu = nmp_mem_kalloc(size);
	if (search_pu)
	{
		memset(search_pu, 0, size);
		search_pu->count = count;
		memcpy(search_pu->result, test,
			count*sizeof(search_result_t));
		nmp_free_search_pu_list();
		g_search_pu_list = search_pu;
	}
}*/

void
nmp_set_search_pu_list(search_array_t *pu_list)
{
	gint size;
	search_pu_list *search_pu = NULL;

	size = sizeof(NmpSearchPuRes) + pu_list->count*sizeof(search_result_t);
	search_pu = nmp_mem_kalloc(size);
	if (search_pu)
	{
		memset(search_pu, 0, size);
		search_pu->count = pu_list->count;
		memcpy(search_pu->result, pu_list->result,
			pu_list->count*sizeof(search_result_t));
		nmp_free_search_pu_list();
		g_search_pu_list = search_pu;
	}
}


search_pu_list*
nmp_get_search_pu_list()
{
	return g_search_pu_list;
}
//:~ End




