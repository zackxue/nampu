
#include "nmp_system_ctrl.h"
#include "nmp_proxy_server.h"
#include "nmp_resolve_host.h"


#define PROXY_VERSION			        0x0200010a


extern hik_service_basic_t hik_srv_basic;
extern dah_service_basic_t dah_srv_basic;
extern hbn_service_basic_t hbn_srv_basic;
extern bsm_service_basic_t bsm_srv_basic;
extern jny_service_basic_t jny_srv_basic;
extern hie_service_basic_t hie_srv_basic;
extern xmt_service_basic_t xmt_srv_basic;
extern tps_service_basic_t tps_srv_basic;
extern plt_service_basic_t plt_srv_basic;
extern cfg_service_basic_t cfg_srv_basic;

int main(int argc, char *argv[])
{
	int i;

	for (i=0; i<argc; i++)
	{
		if (!strcmp(argv[i], "-v"))
		{
			show_info("\n** Proxy Version: %d.%d.%d.%d\n", 
				(PROXY_VERSION & 0xff000000) >> 24, 
				(PROXY_VERSION & 0x00ff0000) >> 16, 
				(PROXY_VERSION & 0x0000ff00) >> 8, 
				(PROXY_VERSION & 0x000000ff) >> 0);
		}
		else if (!strcmp(argv[i], "--version"))
		{
			show_info("\n** Proxy Version: %d.%d.%d.%d\n", 
				(PROXY_VERSION & 0xff000000) >> 24, 
				(PROXY_VERSION & 0x00ff0000) >> 16, 
				(PROXY_VERSION & 0x0000ff00) >> 8, 
				(PROXY_VERSION & 0x000000ff) >> 0);
		}
	}

	init_nmp_xml_msg();
    proxy_running_env_init();
    proxy_resolve_host_init();

#if _DEBUG_
	show_info("\n\n***************************** [%s] Built time: %s %s "
	              "*****************************\n\n\n", 
	              "Debug", __DATE__, __TIME__);
#else
	show_info("\n\n***************************** [%s] Built time: %s %s "
	              "*****************************\n\n\n", 
	               "Release", __DATE__, __TIME__);
#endif

	add_service_template((service_template_t*)&hik_srv_basic);
	add_service_template((service_template_t*)&dah_srv_basic);
	add_service_template((service_template_t*)&hbn_srv_basic);
	//add_service_template((service_template_t*)&bsm_srv_basic);
	//add_service_template((service_template_t*)&jny_srv_basic);
	//add_service_template((service_template_t*)&hie_srv_basic);
	//add_service_template((service_template_t*)&xmt_srv_basic);
	//add_service_template((service_template_t*)&tps_srv_basic);

	add_service_template((service_template_t*)&plt_srv_basic);
	add_service_template((service_template_t*)&cfg_srv_basic);

    proxy_server_loop();
    return 0;
}

