#include "nmp_message.h"


BEGIN_MSG_ID_MAPPING(mss_tbl)
        {"MssRegister" },   /* 0 */
        {"MssHeart" },      /* 1 */
        {"QueryAllHd" },	/* 2 */
        {"AddHdGroup"},		/* 3 */
        {"QueryAllHdGroup"},/* 4 */
        {"QueryHdGroupInfo"},/* 5 */
        {"AddHdToGroup"},	/* 6 */
        {"DelHdFromGroup"},	/* 7 */
        {"DelHdGroup"},		/* 8 */
        {"MssGetGuid"},		/* 9 */
        {"MssGetRecordPolicy"},	/* 10 */
        {"GetHdFormatProgress"},	/* 11 */
        {"MssGetRoute"},	/* 12 */
        {"GuListChange"},	/* 13 */
        {"NotifytRecordPolicyChange"}, /* 14 */
        {"MssGetMds"},		/* 15 */
        {"MssGetMdsIp"},	/* 16 */
        {"GetMssStoreLog"},	/* 17 */
        {"NotifyModifyMss"},/* 18 */
        {"AddOneIpsan"},	/* 19 */
        {"GetIpsanInfo"},	/* 20 */
        {"SetIpsanInfo"},	/* 21 */
        {"GetInitiatorName"},	/* 22 */
        {"SetInitiatorName"},	/* 23 */
        {"GetOneIpsanDetail"},	/* 24 */
        {"DeleteOneIpsan"},	/* 25 */
        {"NotifyMessage"},	/* 26 */
        {"QueryGuRecordStatus"},	/* 27 */
        {"AlarmLinkRecord"},	/* 28 */
        {"RebootMss"},	/* 29 */
END_MSG_ID_MAPPING(mss_tbl)

//:~ End
