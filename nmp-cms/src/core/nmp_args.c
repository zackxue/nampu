#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include "nmp_args.h"
#include "nmp_version.h"

void nmp_print_help_manual()
{
	const char* p_manual_msg =
	"CMS(Center Management Server) Help Manual.\n"
	"\n"
	"\t-H, --help\n"
	"\t\tPrint this help, then exit.\n"
	"\n"
	"\t-h\n"
	"\t\tPrint usage, then exit.\n"
	"\n"
	"\t-v, --version\n"
	"\t\tPrint cms version number, then exit.\n"
	"\n"
	"The end\n";
	printf("%s", p_manual_msg);
}

void nmp_print_usage()
{
	const char* p_usage_msg =
	"Usage: cms [-hvH]\n"
	"Try \"cms --help\" to see more details!\n";
	printf("%s", p_usage_msg);
}

void nmp_print_version()
{
	printf(
		"Center Management Server(CMS)\n"
		"Version %0x  %d.%d.%d.%d  "
		"%s %s\n",
		__CMS_VERSION__,
		((__CMS_VERSION__)>>24)&0xff,
		((__CMS_VERSION__)>>16)&0xff,
		((__CMS_VERSION__)>>8)&0xff,
		(__CMS_VERSION__)&0xff,
		__DATE__,
		__TIME__
	);
}

static struct option optlongs[] =
{
	{"help", 0, NULL, 'H'},
	{"version", 0, NULL, 'v'},
	{NULL, 0, NULL, 'h'},
	{NULL, 0, 0, 0}
};



void nmp_process_main_args(int argc, char* argv[])
{
	int opt;

	while((opt = getopt_long(argc, argv, "hHv",
		optlongs, NULL)) != -1)
	{
		switch(opt)
		{
			case 'h':
			{
				nmp_print_usage();
				exit(0);
			}
			case 'H':
			{
				nmp_print_help_manual();
				exit(0);
			}
			case 'v':
			{
				nmp_print_version();
				exit(0);
			}

			case ':':
			case '?':
			{
				nmp_print_usage();
				exit(1);
			}
		}//switch
	}//while
}











