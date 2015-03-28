#ifndef __NMP_MESSAGE_H__
#define __NMP_MESSAGE_H__

#include "nmp_msg_id_table.h"
#include "message/nmp_msg_struct.h"

#define MESSAGE_RANGE_LOWER                         -1  /* {{Range lower */
#define MESSAGE_REGISTER_CMS                         0   /* Mss register to cms */
#define MESSAGE_KEEPALIVE_CMS                        1   /* Mss register to cms */
#define MESSAGE_QUERY_IDLES_HD						 2
#define MESSAGE_ADD_HD_GROUP						 3
#define MESSAGE_QUERY_HD_GROUPS						 4	/* Query all disk groups */		
#define MESSAGE_QUERY_HD_GROUP_INFO					 5	/* Query disks of the group */
#define MESSAGE_ADD_GROUP_DISK						 6
#define MESSAGE_DEL_GROUP_DISK						 7
#define MESSAGE_DEL_HD_GROUP						 8
#define MESSAGE_QUERY_GUIDS							 9
#define MESSAGE_QUERY_POLICY						 10
#define MESSAGE_HDFMT_PROGRESS						 11
#define MESSAGE_QUERY_URI							 12
#define MESSAGE_GULIST_CHANGED						 13
#define MESSAGE_GUPOLICY_CHANGED					 14
#define MESSAGE_GET_ALL_MDS							 15
#define MESSAGE_GET_MDS_IP							 16
#define MESSAGE_GET_RECORDS_LIST					 17
#define MESSAGE_NOTIFY_MSS_MODE						 18
#define MESSAGE_ADD_ONE_IPSAN							19
#define MESSAGE_GET_IPSANS								20
#define MESSAGE_SET_IPSANS								21
#define MESSAGE_GET_INITIATOR_NAME					22
#define MESSAGE_SET_INITIATOR_NAME					23
#define MESSAGE_GET_ONE_IPSAN_DETAIL					24
#define MESSAGE_DEL_ONE_IPSAN							25
#define MESSAGE_NOTIFY_MESSAGE						26
#define MESSAGE_QUERY_RECORD_STATUS					27
#define MESSAGE_ALARM_LINK_RECORD					28
#define MESSAGE_SYSTEM_REBOOT						29

#define MSG_INTERNAL_BASE                           255 /* Base number of internal Msg */

#define MESSAGE_GUS_DIFFSET_NOTIFY					(MSG_INTERNAL_BASE + 1)
#define MESSAGE_START_RECORD						(MSG_INTERNAL_BASE + 2)
#define MESSAGE_STOP_RECORD							(MSG_INTERNAL_BASE + 3)
#define MESSAGE_MDS_DIFFSET_NOTIFY					(MSG_INTERNAL_BASE + 4)
#define MESSAGE_REGISTED_NOTIFY						(MSG_INTERNAL_BASE + 5)

#endif  //__NMP_MESSAGE_H__
