/*
 *	@author:	zyt
 *	@time:	2013/05/04
 */
#ifndef __NMP_LOG_DEFS_H__
#define __NMP_LOG_DEFS_H__


//#define LOG_USER_NAME_LEN		64
//#define LOG_CHILD_TYPE_LEN		64
#define LOG_MAX_LOG_NUM			10000
#define LOG_DEL_LOG_NUM_PERTIME	1000


/*
 * log level
 */
typedef enum
{
	LOG_LEVEL_0 = 1 << 0,
	LOG_LEVEL_1 = 1 << 1,
	LOG_LEVEL_2 = 1 << 2,
	LOG_LEVEL_3 = 1 << 3
} LOG_LEVEL;


/*
 * log operate type
 * must be in order to increase.
 */
#define LOG_BSS_LOGIN					1
#define LOG_ADD_ADMIN					2
#define LOG_MODIFY_ADMIN				3
#define LOG_DEL_ADMIN					4
#define LOG_ADD_USER_GROUP			5
#define LOG_MODIFY_USER_GROUP			6
#define LOG_DEL_USER_GROUP			7
#define LOG_ADD_USER					8
#define LOG_MODIFY_USER				9
#define LOG_DEL_USER					10
#define LOG_MODIFY_DOMAIN				11
#define LOG_ADD_DOMAIN					12	//not deal
#define LOG_DEL_DOMAIN					13	//not deal
#define LOG_ADD_MODIFY_AREA			14
#define LOG_DEL_AREA					15
#define LOG_ADD_PU						16
#define LOG_MODIFY_PU					17
#define LOG_DEL_PU						18
#define LOG_ADD_GU						19
#define LOG_MODIFY_GU					20
#define LOG_DEL_GU						21
#define LOG_ADD_MDS					22
#define LOG_MODIFY_MDS					23
#define LOG_DEL_MDS					24
#define LOG_ADD_MDS_IP					25
#define LOG_DEL_MDS_IP					26
#define LOG_ADD_MSS					27
#define LOG_MODIFY_MSS					28
#define LOG_DEL_MSS						29
#define LOG_RECORD_POLICY_CONFIG		30	//ÅäÖÃÂ¼Ïñ²ßÂÔ
#define LOG_ADD_MODIFY_MANUFACTURER	31
#define LOG_DEL_MANUFACTURER			32	//not use
#define LOG_ADD_GU_TO_USER			33
#define LOG_SET_TIME					34
#define LOG_DATABASE_BACKUP			35	//wait test
#define LOG_DATABASE_IMPORT			36	//wait test
#define LOG_SET_NETWORK_CONFIG		37
#define LOG_ADD_HD_GROUP				38
#define LOG_ADD_HD						39
#define LOG_DEL_HD						40
#define LOG_REBOOT_MSS					41	//wait test
#define LOG_DEL_HD_GROUP				42
#define LOG_ADD_DEFENCE_AREA			43
#define LOG_MODIFY_DEFENCE_AREA		44	//not use
#define LOG_DEL_DEFENCE_AREA			45
#define LOG_ADD_DEFENCE_MAP			46
#define LOG_DEL_DEFENCE_MAP			47
#define LOG_ADD_DEFENCE_GU			48
#define LOG_MODIFY_DEFENCE_GU			49
#define LOG_DEL_DEFENCE_GU			50
#define LOG_SET_MAP_HREF				51
#define LOG_MODIFY_MAP_HREF			52
#define LOG_DEL_MAP_HREF				53
#define LOG_PLATFORM_UPGRADE			54	//wait test
#define LOG_DEL_ALARM					55
#define LOG_SET_DEL_ALARM_POLICY		56
#define LOG_ADD_TW						57
#define LOG_MODITY_TW					58
#define LOG_DEL_TW						59
#define LOG_ADD_SCREEN					60
#define LOG_MODIFY_SCREEN				61
#define LOG_DEL_SCREEN					62
#define LOG_ADD_TOUR					63
#define LOG_MODIFY_TOUR				64
#define LOG_DEL_TOUR					65
#define LOG_ADD_TOUR_STEP				66
#define LOG_ADD_GROUP					67
#define LOG_MODIFY_GROUP				68
#define LOG_DEL_GROUP					69
#define LOG_ADD_GROUP_STEP			70
#define LOG_MODIFY_GROUP_STEP			71
#define LOG_DEL_GROUP_STEP				72
#define LOG_CONFIG_GROUP_STEP			73
#define LOG_MODIFY_GROUP_STEP_INFO	74
#define LOG_DEL_GROUP_STEP_INFO		75
#define LOG_LINK_TIME_POLICY_CONFIG	76
#define LOG_MODIFY_LINK_TIME_POLICY	77
#define LOG_DEL_LINK_TIME_POLICY		78
#define LOG_LINK_RECORD_CONFIG		79
#define LOG_MODIFY_LINK_RECORD		80
#define LOG_DEL_LINK_RECORD			81
#define LOG_LINK_IO_CONFIG				82
#define LOG_MODIFY_LINK_IO				83
#define LOG_DEL_LINK_IO					84
#define LOG_LINK_SNAPSHOT_CONFIG		85
#define LOG_MODIFY_LINK_SNAPSHOT		86
#define LOG_DEL_LINK_SNAPSHOT			87
#define LOG_LINK_PRESET_CONFIG			88
#define LOG_MODIFY_LINK_PRESET			89
#define LOG_DEL_LINK_PRESET			90
#define LOG_LINK_STEP_CONFIG			91
#define LOG_MODIFY_LINK_STEP			92	//not use
#define LOG_DEL_LINK_STEP				93
#define LOG_AUTO_ADD_PU				94
#define LOG_SET_INITIATOR_NAME			95	//wait test
#define LOG_ADD_ONE_IPSAN				96	//wait test
#define LOG_DEL_ONE_IPSAN				97	//wait test
#define LOG_LINK_MAP_CONFIG				98
#define LOG_MODIFY_LINK_MAP				99
#define LOG_DEL_LINK_MAP				100
#define LOG_ADD_TW_TO_USER				101
#define LOG_ADD_TOUR_TO_USER			102

#endif	//__NMP_LOG_DEFS_H__