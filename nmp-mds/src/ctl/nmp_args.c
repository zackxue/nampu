#include <stdlib.h>
#include <getopt.h>
#include <glib.h>
#include <stdio.h>
#include "nmp_version.h"


static __inline__ void
nmp_mds_print_usage( void )
{
	const gchar *usage = 
	"Usage: mds\n"
	"    Enjoy it!\n";

	printf(usage);
}


static __inline__ void
nmp_mds_print_version( void )
{
	printf(
		JPF_MEDIA_SERVER_LOGO
		"\nVersion %d.%d.%d.%d\n",
		(JPF_MDS_VERSION_NUMBER & 0xff000000) >> 24,
		(JPF_MDS_VERSION_NUMBER & 0x00ff0000) >> 16,
		(JPF_MDS_VERSION_NUMBER & 0x0000ff00) >> 8,
		(JPF_MDS_VERSION_NUMBER & 0x000000ff)
	);
}


static struct option opt_longs[] = 
{
	{"version", 0, NULL, 'v'},
	{NULL, 0, NULL, 'h'},
	{NULL, 0, 0, 0}
};


void nmp_mds_main_args(gint argc, gchar* argv[])
{
	int opt;

	while((opt = getopt_long(argc, argv, "hv", 
		opt_longs, NULL)) != -1)
	{
		switch(opt)
		{
		case 'h':
			nmp_mds_print_usage();
			exit(0);

		case 'v':
			nmp_mds_print_version();
			exit(0);
			break;

		default:
			break;
		}
	}
}


//:~ End
