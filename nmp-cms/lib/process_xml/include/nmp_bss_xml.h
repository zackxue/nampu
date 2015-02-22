/********************************************************************
 * jpf_bss.h  - deal xml of bss, parse and create xml
 * Function£ºparse or create xml relate to bss.
 * Author:yangy
 * Description:users can add parse or create message of bss,define
 *             struct about bss information
 * History:
 * 2011.06.01 - Yang Ying, initiate to create;
 * 2011.06.07 - Yang ying modify
 ********************************************************************/

#ifndef __NMP_BSS_XML__
#define __NMP_BSS_XML__

#define BSS_LOGIN					"AdministratorLogin"
#define BSS_LOGIN_RES				"AdministratorLoginResponse"
#define BSS_HEART					"BssKeepAlive"
#define BSS_HEART_RES				"BssKeepAliveResponse"

#define ADD_ADMIN					"AddAdministrator"
#define ADD_ADMIN_RES				"AddAdministratorResponse"
#define MODIFY_ADMIN				"ModifyAdministratorPassword"
#define MODIFY_ADMIN_RES			"ModifyAdministratorPasswordResponse"
#define DEL_ADMIN					"DelAdministrator"
#define DEL_ADMIN_RES				"DelAdministratorResponse"
#define QUERY_ADMIN				"QueryAdministrator"
#define QUERY_ADMIN_RES			"QueryAdministratorResponse"
#define VALIDATA_ADMIN				"ValidateAdministrator"
#define VALIDATA_ADMIN_RES			"ValidateAdministratorResponse"

#define ADD_USER_GROUP				"AddUserGroup"
#define ADD_USER_GROUP_RES		"AddUserGroupResponse"
#define MODIFY_USER_GROUP			"ModifyUserGroup"
#define MODIFY_USER_GROUP_RES		"ModifyUserGroupResponse"
#define DEL_USER_GROUP				"DelUserGroup"
#define DEL_USER_GROUP_RES             "DelUserGroupResponse"
#define QUERY_USER_GROUP			"QueryUserGroup"
#define QUERY_USER_GROUP_RES         "QueryUserGroupResponse"
#define VALIDATA_USER_GROUP           "ValidateUserGroup"
#define VALIDATA_USER_GROUP_RES	"ValidateUserGroupResponse"

#define ADD_USER					"AddUser"
#define ADD_USER_RES                         "AddUserResponse"
#define MODIFY_USER                            "ModifyUser"
#define MODIFY_USER_RES                    "ModifyUserResponse"
#define DEL_USER                                  "DelUser"
#define DEL_USER_RES                          "DelUserResponse"
#define QUERY_USER                              "QueryUser"
#define QUERY_USER_RES                      "QueryUserResponse"
#define VALIDATA_USER                        "ValidateUser"
#define VALIDATA_USER_RES                "ValidateUserResponse"

#define QUERY_DOMAIN                        "QueryDomain"
#define QUERY_DOMAIN_RES                "QueryDomainResponse"
#define MODIFY_DOMAIN                      "ModifyDomain"
#define MODIFY_DOMAIN_RES              "ModifyDomainResponse"

#define ADD_MODIFY_AREA                  "AddOrModifyArea"
#define ADD_MODIFY_AREA_RES          "AddOrModifyAreaResponse"
#define DEL_AREA                                 "DelArea"
#define DEL_AREA_RES                         "DelAreaResponse"
#define QUERY_AREA                             "QueryArea"
#define QUERY_AREA_RES                     "QueryAreaResponse"

#define ADD_PU                                   "AddPu"
#define ADD_PU_RES                            "AddPuResponse"
#define DEL_PU                                     "DelPu"
#define DEL_PU_RES                             "DelPuResponse"
#define QUERY_PU                                "QueryPu"
#define QUERY_PU_RES                        "QueryPuResponse"
#define VALIDATA_PU                          "ValidatePu"
#define VALIDATA_PU_RES                  "ValidatePuResponse"
#define MODIFY_PU                          "ModifyPu"
#define MODIFY_PU_RES                      "ModifyPuResponse"

#define ADD_GU                                   "AddGu"
#define ADD_GU_RES                           "AddGuResponse"
#define MODIFY_GU                             "ModifyGu"
#define MODIFY_GU_RES                     "ModifyGuResponse"
#define DEL_GU                                   "DelGu"
#define DEL_GU_RES                           "DelGuResponse"
#define QUERY_GU                               "QueryGu"
#define QUERY_GU_RES                       "QueryGuResponse"
#define VALIDATA_GU                         "ValidateGu"
#define VALIDATA_GU_RES                 "ValidateGuResponse"

#define ADD_MDS                                 "AddMdu"
#define ADD_MDS_RES                          "AddMduResponse"
#define MODIFY_MDS                            "ModifyMdu"
#define MODIFY_MDS_RES                    "ModifyMduResponse"
#define DEL_MDS                                  "DelMdu"
#define DEL_MDS_RES                          "DelMduResponse"
#define QUERY_MDS                              "QueryMdu"
#define QUERY_MDS_RES                      "QueryMduResponse"

#define ADD_MDS_IP                             "AddMduIp"
#define ADD_MDS_IP_RES                     "AddMduIpResponse"
#define DEL_MDS_IP                              "DelMduIp"
#define DEL_MDS_IP_RES                       "DelMduIpResponse"
#define QUERY_MDS_IP                         "QueryMduIp"
#define QUERY_MDS_IP_RES                 "QueryMduIpResponse"

#define ADD_MSS                                 "AddMss"
#define ADD_MSS_RES                          "AddMssResponse"
#define MODIFY_MSS                            "ModifyMss"
#define MODIFY_MSS_RES                    "ModifyMssResponse"
#define DEL_MSS						"DelMss"
#define DEL_MSS_RES                                   "DelMssResponse"
#define QUERY_MSS                                      "QueryMss"
#define QUERY_MSS_RES                              "QueryMssResponse"
#define QUERY_RECORD_POLICY                   "QueryRecordPolicy"
#define QUERY_RECORD_POLICY_RES           "QueryRecordPolicyResponse"
#define RECORD_POLICY_CONFIG                  "RecordPolicyConfig"
#define RECORD_POLICY_CONFIG_RES          "RecordPolicyConfigResponse"


#define ADD_MODIFY_MANUFACTURER              "AddOrModifyManufacturer"
#define ADD_MODIFY_MANUFACTURER_RES       "AddOrModifyManufacturerResponse"
#define DEL_MANUFACTURER                              "DelManufacturer"
#define DEL_MANUFACTURER_RES                      "DelManufacturerResponse"
#define QUERY_MANUFACTURER                          "QueryManufacturer"
#define QUERY_MANUFACTURER_RES                  "QueryManufacturerResponse"

#define ADD_GU_TO_USER                                  "AddGuToUser"
#define ADD_GU_TO_USER_RES                          "AddGuToUserResponse"
#define MODIFY_GU_TO_USER                             "ModifyGuToUser"
#define MODIFY_GU_TO_USER_RES                     "ModifyGuToUserResponse"
#define DEL_GU_FROM_USER                               "DelGuFromUser"
#define DEL_GU_FROM_USER_RES                       "DelGuFromUserResponse"
#define QUERY_USER_OWN_GU                            "QueryUserOwnGu"
#define QUERY_USER_OWN_GU_RES                    "QueryUserOwnGuResponse"

#define ADD_TW_TO_USER                                  "AddTwToUser"
#define ADD_TW_TO_USER_RES                          "AddTwToUserResponse"
#define QUERY_USER_OWN_TW                            "QueryUserOwnTw"
#define QUERY_USER_OWN_TW_RES                    "QueryUserOwnTwResponse"

#define ADD_TOUR_TO_USER                                  "AddTourToUser"
#define ADD_TOUR_TO_USER_RES                          "AddTourToUserResponse"
#define QUERY_USER_OWN_TOUR                            "QueryUserOwnTour"
#define QUERY_USER_OWN_TOUR_RES                    "QueryUserOwnTourResponse"

#define QUERY_SERVER_TIME                              "QueryServerTime"
#define QUERY_SERVER_TIME_RES                      "QueryServerTimeResponse"
#define SET_SERVER_TIME                                  "SetServerTime"
#define SET_SERVER_TIME_RES                          "SetServerTimeResponse"
#define DATABASE_BACKUP                                "DatabaseBackup"
#define DATABASE_BACKUP_RES                        "DatabaseBackupResponse"
#define DATABASE_IMPORT                                 "DatabaseImport"
#define DATABASE_IMPORT_RES                         "DatabaseImportResponse"

#define ADD_HD_GROUP                                       "AddHdGroup"
#define ADD_HD_GROUP_RES                               "AddHdGroupResponse"
#define ADD_HD_TO_GROUP                                 "AddHdToGroup"
#define ADD_HD_TO_GROUP_RES                         "AddHdToGroupResponse"
#define DEL_HD_FROM_GROUP                             "DelHdFromGroup"
#define DEL_HD_FROM_GROUP_RES                     "DelHdFromGroupResponse"
#define REBOOT_MSS                             "RebootMss"
#define REBOOT_MSS_RES                     "RebootMssResponse"
#define QUERY_ALL_HD_GROUP                           "QueryAllHdGroup"
#define QUERY_ALL_HD_GROUP_RES                   "QueryAllHdGroupResponse"
#define QUERY_HD_GROUP_INFO                         "QueryHdGroupInfo"
#define QUERY_HD_GROUP_INFO_RES                 "QueryHdGroupInfoResponse"
#define QUERY_ALL_HD                                        "QueryAllHd"
#define QUERY_ALL_HD_RES                                "QueryAllHdResponse"
#define DEL_HD_GROUP                                        "DelHdGroup"
#define DEL_HD_GROUP_RES                                "DelHdGroupResponse"
#define GET_HD_FORMAT_PROGRESS                   "GetHdFormatProgress"
#define GET_HD_FORMAT_PROGRESS_RES           "GetHdFormatProgressResponse"
#define QUERY_GU_RECORD_STATUS                  "QueryGuRecordStatus"
#define QUERY_GU_RECORD_STATUS_RES           "QueryGuRecordStatusResponse"

#define ADD_DEFENCE_AREA                                "AddDefenceArea"
#define ADD_DEFENCE_AREA_RES                        "AddDefenceAreaResponse"
#define MODIFY_DEFENCE_AREA                          "ModifyDefenceArea"
#define MODIFY_DEFENCE_AREA_RES                  "ModifyDefenceAreaResponse"
#define DEL_DEFENCE_AREA                                 "DelDefenceArea"
#define DEL_DEFENCE_AREA_RES                         "DelDefenceAreaResponse"
#define QUERY_DEFENCE_AREA                             "QueryAllDefence"
#define QUERY_DEFENCE_AREA_RES                     "QueryAllDefenceResponse"
#define ADD_DEFENCE_MAP                                  "AddDefenceMap"
#define ADD_DEFENCE_MAP_RES                          "AddDefenceMapResponse"
#define DEL_DEFENCE_MAP                                   "DelDefenceMap"
#define DEL_DEFENCE_MAP_RES                           "DelDefenceMapResponse"
#define QUERY_DEFENCE_MAP                              "QueryDefenceMap"
#define QUERY_DEFENCE_MAP_RES                       "QueryDefenceMapResponse"
#define ADD_DEFENCE_GU                                     "AddDefenceGu"
#define ADD_DEFENCE_GU_RES                             "AddDefenceGuResponse"
#define MODIFY_DEFENCE_GU                               "ModifyDefenceGu"
#define MODIFY_DEFENCE_GU_RES                       "ModifyDefenceGuResponse"
#define DEL_DEFENCE_GU                                      "DelDefenceGu"
#define DEL_DEFENCE_GU_RES                              "DelDefenceGuResponse"
#define QUERY_DEFENCE_GU                                 "QueryDefenceGu"
#define QUERY_DEFENCE_GU_RES                         "QueryDefenceGuResponse"
#define SET_MAP_HREF                                         "SetMapHref"
#define SET_MAP_HREF_RES                                 "SetMapHrefResponse"
#define MODIFY_MAP_HREF                                   "ModifyMapHref"
#define MODIFY_MAP_HREF_RES                           "ModifyMapHrefResponse"
#define DEL_MAP_HREF                                          "DelMapHref"
#define DEL_MAP_HREF_RES                                  "DelMapHrefResponse"
#define QUERY_MAP_HREF                                     "QueryMapHref"
#define QUERY_MAP_HREF_RES                             "QueryMapHrefResponse"
#define QUERY_ALARM                                           "QueryAlarm"
#define QUERY_ALARM_RES                                   "QueryAlarmResponse"
#define DEL_ALARM                                                "DelAlarm"
#define DEL_ALARM_RES                                        "DelAlarmResponse"
#define QUERY_DEL_ALARM_POLICY                       "QueryAutoDelAlarmPolicy"
#define QUERY_DEL_ALARM_POLICY_RES               "QueryAutoDelAlarmPolicyResponse"
#define SET_DEL_ALARM_POLICY                           "SetAutoDelAlarmPolicy"
#define SET_DEL_ALARM_POLICY_RES                   "SetAutoDelAlarmPolicyResponse"
#define PLATFORM_UPGRADE                                 "PlatformUpgrade"
#define PLATFORM_UPGRADE_RES				"PlatformUpgradeResponse"
#define ADD_TW								"AddTw"
#define ADD_TW_RES							"AddTwResponse"
#define MODIFY_TW							"ModifyTw"
#define MODIFY_TW_RES						"ModifyTwResponse"
#define DEL_TW								"DelTw"
#define DEL_TW_RES							"DelTwResponse"
#define QUERY_TW							"QueryTw"
#define QUERY_TW_RES						"QueryTwResponse"
#define ADD_SCREEN							"AddScreen"
#define ADD_SCREEN_RES						"AddScreenResponse"
#define MODIFY_SCREEN						"ModifyScreen"
#define MODIFY_SCREEN_RES					"ModifyScreenResponse"
#define DEL_SCREEN 							"DelScreen"
#define DEL_SCREEN_RES						"DelScreenResponse"
#define QUERY_SCREEN						"QueryScreen"
#define QUERY_SCREEN_RES					"QueryScreenResponse"
#define QUERY_SCR_DIV						"QueryScreenDivision"
#define QUERY_SCR_DIV_RES					"QueryScreenDivisionResponse"
#define ADD_TOUR							"AddTour"
#define ADD_TOUR_RES						"AddTourResponse"
#define MODIFY_TOUR						"ModifyTour"
#define MODIFY_TOUR_RES					"ModifyTourResponse"
#define DEL_TOUR							"DelTour"
#define DEL_TOUR_RES						"DelTourResponse"
#define QUERY_TOUR							"QueryTour"
#define QUERY_TOUR_RES						"QueryTourResponse"
#define ADD_TOUR_STEP						"AddTourStep"
#define ADD_TOUR_STEP_RES                            "AddTourStepResponse"
#define QUERY_TOUR_STEP					"QueryTourStep"
#define QUERY_TOUR_STEP_RES                        "QueryTourStepResponse"
#define ADD_GROUP							"AddGroup"
#define ADD_GROUP_RES						"AddGroupResponse"
#define MODIFY_GROUP						"ModifyGroup"
#define MODIFY_GROUP_RES					"ModifyGroupResponse"
#define DEL_GROUP							"DelGroup"
#define DEL_GROUP_RES						"DelGroupResponse"
#define QUERY_GROUP						"QueryGroup"
#define QUERY_GROUP_RES					"QueryGroupResponse"
#define ADD_GROUP_STEP						"AddGroupStep"
#define ADD_GROUP_STEP_RES					"AddGroupStepResponse"
#define MODIFY_GROUP_STEP					"ModifyGroupStep"
#define MODIFY_GROUP_STEP_RES				"ModifyGroupStepResponse"
#define DEL_GROUP_STEP						"DelGroupStep"
#define DEL_GROUP_STEP_RES					"DelGroupStepResponse"
#define QUERY_GROUP_STEP					"QueryGroupStep"
#define QUERY_GROUP_STEP_RES				"QueryGroupStepResponse"
#define CONFIG_GROUP_STEP					"ConfigGroupStep"
#define CONFIG_GROUP_STEP_RES				"ConfigGroupStepResponse"
#define MODIFY_GROUP_STEP_INFO			"ModifyGroupStepInfo"
#define MODIFY_GROUP_STEP_INFO_RES		"ModifyGroupStepInfoResponse"
#define DEL_GROUP_STEP_INFO				"DelGroupStepInfo"
#define DEL_GROUP_STEP_INFO_RES			"DelGroupStepInfoResponse"
#define QUERY_GROUP_STEP_INFO				"QueryGroupStepInfo"
#define QUERY_GROUP_STEP_INFO_RES			"QueryGroupStepInfoResponse"
#define QUERY_GROUP_STEP_DIV				"QueryGroupStepDiv"
#define QUERY_GROUP_STEP_DIV_RES			"QueryGroupStepDivResponse"
#define GET_NETINTERFACE_CONFIG                  "GetNetInterfaceConfig"
#define GET_NETINTERFACE_CONFIG_RES		"GetNetInterfaceConfigResponse"
#define GET_NETWORK_CONFIG				"GetNetworkConfig"
#define GET_NETWORK_CONFIG_RES			"GetNetworkConfigResponse"
#define SET_NETWORK_CONFIG				"SetNetworkConfig"
#define SET_NETWORK_CONFIG_RES			"SetNetworkConfigResponse"
#define LINK_TIME_POLICY_CONFIG			"LinkTimePolicyConfig"
#define LINK_TIME_POLICY_CONFIG_RES		"LinkTimePolicyConfigResponse"
#define MODIFY_LINK_TIME_POLICY			"ModifyLinkTimePolicy"
#define MODIFY_LINK_TIME_POLICY_RES		"ModifyLinkTimePolicyResponse"
#define QUERY_LINK_TIME_POLICY				"QueryLinkTimePolicy"
#define QUERY_LINK_TIME_POLICY_RES		"QueryLinkTimePolicyResponse"
#define DEL_LINK_TIME_POLICY			"DelLinkTimePolicy"
#define DEL_LINK_TIME_POLICY_RES		"DelLinkTimePolicyResponse"
#define LINK_RECORD						"LinkRecordConfig"
#define LINK_RECORD_RES					"LinkRecordConfigResponse"
#define MODIFY_LINK_RECORD				"ModifyLinkRecord"
#define MODIFY_LINK_RECORD_RES		"ModifyLinkRecordResponse"
#define QUERY_LINK_RECORD				"QueryLinkRecord"
#define QUERY_LINK_RECORD_RES			"QueryLinkRecordResponse"
#define DEL_LINK_RECORD					"DelLinkRecord"
#define DEL_LINK_RECORD_RES			"DelLinkRecordResponse"
#define LINK_IO						"LinkIOConfig"
#define LINK_IO_RES					"LinkIOConfigResponse"
#define MODIFY_LINK_IO				"ModifyLinkIO"
#define MODIFY_LINK_IO_RES		"ModifyLinkIOResponse"
#define QUERY_LINK_IO				"QueryLinkIO"
#define QUERY_LINK_IO_RES			"QueryLinkIOResponse"
#define DEL_LINK_IO				"DelLinkIO"
#define DEL_LINK_IO_RES			"DelLinkIOResponse"
#define LINK_SNAPSHOT						"LinkSnapshotConfig"
#define LINK_SNAPSHOT_RES					"LinkSnapshotConfigResponse"
#define MODIFY_LINK_SNAPSHOT			"ModifyLinkSnapshot"
#define MODIFY_LINK_SNAPSHOT_RES		"ModifyLinkSnapshotResponse"
#define QUERY_LINK_SNAPSHOT				"QueryLinkSnapshot"
#define QUERY_LINK_SNAPSHOT_RES			"QueryLinkSnapshotResponse"
#define DEL_LINK_SNAPSHOT					"DelLinkSnapshot"
#define DEL_LINK_SNAPSHOT_RES			"DelLinkSnapshotResponse"
#define QUERY_SERVER_RESOURCE_INFO			"QueryServerResourceInfo"
#define QUERY_SERVER_RESOURCE_INFO_RES		"QueryServerResourceInfoResponse"
#define SEARCH_PU							"SearchPu"
#define SEARCH_PU_RES						"SearchPuResponse"
#define GET_SEARCHED_PUS					"GetSearchedPus"
#define GET_SEARCHED_PUS_RES				"GetSearchedPusResponse"
#define QUERY_CMS_ALL_IPS			"QueryCmsAllIp"
#define QUERY_CMS_ALL_IPS_RES		"QueryCmsAllIpResponse"
#define QUERY_TW_AUTH_INFO			"QueryTwAuthInfo"
#define QUERY_TW_AUTH_INFO_RES		"QueryTwAuthInfoResponse"
#define AUTO_ADD_PU                         "AutoAddPu"
#define AUTO_ADD_PU_RES                  "AutoAddPuResponse"
#define GET_NEXT_PUNO                         "GetNextPuNo"
#define GET_NEXT_PUNO_RES                  "GetNextPuNoResponse"
#define GET_INIT_NAME                         "GetInitiatorName"
#define GET_INIT_NAME_RES                  "GetInitiatorNameResponse"
#define SET_INIT_NAME                         "SetInitiatorName"
#define SET_INIT_NAME_RES                  "SetInitiatorNameResponse"
#define GET_IPSAN_INFO                         "GetIpsanInfo"
#define GET_IPSAN_INFO_RES                  "GetIpsanInfoResponse"
#define ADD_ONE_IPSAN                         "AddOneIpsan"
#define ADD_ONE_IPSAN_RES                  "AddOneIpsanResponse"
#define DELETE_ONE_IPSAN                         "DeleteOneIpsan"
#define DELETE_ONE_IPSAN_RES                  "DeleteOneIpsanResponse"
#define GET_ONE_IPSAN_DETAIL                  "GetOneIpsanDetail"
#define GET_ONE_IPSAN_DETAIL_RES           "GetOneIpsanDetailResponse"
#define LINK_PRESET						"LinkPresetConfig"
#define LINK_PRESET_RES					"LinkPresetConfigResponse"
#define MODIFY_LINK_PRESET				"ModifyLinkPreset"
#define MODIFY_LINK_PRESET_RES		"ModifyLinkPresetResponse"
#define QUERY_LINK_PRESET				"QueryLinkPreset"
#define QUERY_LINK_PRESET_RES			"QueryLinkPresetResponse"
#define DEL_LINK_PRESET				"DelLinkPreset"
#define DEL_LINK_PRESET_RES			"DelLinkPresetResponse"
#define LINK_STEP						"LinkStepConfig"
#define LINK_STEP_RES					"LinkStepConfigResponse"
#define MODIFY_LINK_STEP				"ModifyLinkStep"
#define MODIFY_LINK_STEP_RES		"ModifyLinkStepResponse"
#define QUERY_LINK_STEP				"QueryLinkStep"
#define QUERY_LINK_STEP_RES			"QueryLinkStepResponse"
#define DEL_LINK_STEP				"DelLinkStep"
#define DEL_LINK_STEP_RES			"DelLinkStepResponse"
#define LINK_MAP						"LinkMapConfig"
#define LINK_MAP_RES					"LinkMapConfigResponse"
#define MODIFY_LINK_MAP				"ModifyLinkMap"
#define MODIFY_LINK_MAP_RES		"ModifyLinkMapResponse"
#define QUERY_LINK_MAP				"QueryLinkMap"
#define QUERY_LINK_MAP_RES			"QueryLinkMapResponse"
#define DEL_LINK_MAP				"DelLinkMap"
#define DEL_LINK_MAP_RES			"DelLinkMapResponse"
#define QUERY_LOG					"QueryLog"
#define QUERY_LOG_RES				"QueryLogResponse"
#define QUERY_ALARM_LINK_AUTH_INFO			"QueryAlarmLinkAuthInfo"
#define QUERY_ALARM_LINK_AUTH_INFO_RES		"QueryAlarmLinkAuthInfoResponse"
#define QUERY_AREA_DEV_ONLINE_RATE   		"QueryAreaDevOnlineRate"
#define QUERY_AREA_DEV_ONLINE_RATE_RES		"QueryAreaDevOnlineRateResponse"
#define VALIDATA_GU_MAP           "ValidateGuMap"
#define VALIDATA_GU_MAP_RES	"ValidateGuMapResponse"
#define ADD_IVS                                 "AddIvs"
#define ADD_IVS_RES                          "AddIvsResponse"
#define MODIFY_IVS                            "ModifyIvs"
#define MODIFY_IVS_RES                    "ModifyIvsResponse"
#define DEL_IVS                                  "DelIvs"
#define DEL_IVS_RES                          "DelIvsResponse"
#define QUERY_IVS                              "QueryIvs"
#define QUERY_IVS_RES                      "QueryIvsResponse"
#define GET_SERVER_FALG				"GetServerFlag"
#define GET_SERVER_FALG_RES			"GetServerFlagResponse"
#define GET_MDS_CONFIG					"GetMdsConfig"
#define GET_MDS_CONFIG_RES				"GetMdsConfigResponse"
#define SET_MDS_CONFIG					"SetMdsConfig"
#define SET_MDS_CONFIG_RES				"SetMdsConfigResponse"
#define GET_MDS_STATE					"GetMdsState"
#define GET_MDS_STATE_RES				"GetMdsStateResponse"
#define GET_MSS_CONFIG					"GetMssConfig"
#define GET_MSS_CONFIG_RES				"GetMssConfigResponse"
#define SET_MSS_CONFIG					"SetMssConfig"
#define SET_MSS_CONFIG_RES				"SetMssConfigResponse"
#define GET_MSS_STATE					"GetMssState"
#define GET_MSS_STATE_RES				"GetMssStateResponse"
#define GET_IVS_CONFIG					"GetIvsConfig"
#define GET_IVS_CONFIG_RES				"GetIvsConfigResponse"
#define SET_IVS_CONFIG					"SetIvsConfig"
#define SET_IVS_CONFIG_RES				"SetIvsConfigResponse"
#define GET_IVS_STATE					"GetIvsState"
#define GET_IVS_STATE_RES				"GetIvsStateResponse"
#define CMS_SHUTDOWN					"CmsShutdown"
#define CMS_SHUTDOWN_RES				"CmsShutdownResponse"
#define CMS_REBOOT						"CmsReboot"
#define CMS_REBOOT_RES					"CmsRebootResponse"
#define ADD_AMS							"AddAms"
#define ADD_AMS_RES					"AddAmsResponse"
#define MODIFY_AMS						"ModifyAms"
#define MODIFY_AMS_RES					"ModifyAmsResponse"
#define DEL_AMS							"DelAms"
#define DEL_AMS_RES						"DelAmsResponse"
#define QUERY_AMS						"QueryAms"
#define QUERY_AMS_RES					"QueryAmsResponse"
#define QUERY_AMS_PU					"QueryAmsPu"
#define QUERY_AMS_PU_RES				"QueryAmsPuResponse"
#define MODIFY_AMS_PU					"ModifyAmsPu"
#define MODIFY_AMS_PU_RES				"ModifyAmsPuResponse"


JpfMsgInfo *
jpf_parse_bss_login(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_bss_login_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);
JpfMsgInfo *
jpf_parse_bss_heart(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_bss_heart_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_add_admin(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_add_admin_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_modify_admin(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_modify_admin_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_del_admin(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);
int
jpf_create_del_admin_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_query_admin(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_query_admin_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo * jpf_parse_validata_admin(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int jpf_create_validata_admin_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);


JpfMsgInfo *
jpf_parse_add_user_group(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_add_user_group_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_validata_user_group(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_validata_user_group_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_query_user_group(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_query_user_group_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_modify_user_group(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_modify_user_group_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_del_user_group(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_del_user_group_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_add_user(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_add_user_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_validata_user(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_validata_user_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_query_user(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_query_user_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_modify_user(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_modify_user_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_del_user(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_del_user_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_query_domain(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_query_domain_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

 JpfMsgInfo *
jpf_parse_modify_domain(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

 int
jpf_create_modify_domain_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_add_area(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_add_area_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_validata_area(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_validata_area_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_query_area(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_query_area_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_add_modify_area(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_add_modify_area_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_del_area(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_del_area_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_add_pu(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_add_pu_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_validata_pu(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_validata_pu_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_query_pu(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_query_pu_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_modify_pu(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_modify_pu_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_del_pu(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_del_pu_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_move_pu(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_move_pu_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_add_gu(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_add_gu_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_query_gu(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_query_gu_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_modify_gu(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_modify_gu_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_del_gu(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_del_gu_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_add_mds(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_add_mds_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_query_mds(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_query_mds_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_modify_mds(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_modify_mds_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_del_mds(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_del_mds_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_add_mds_ip(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_add_mds_ip_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_query_mds_ip(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_query_mds_ip_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_del_mds_ip(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_del_mds_ip_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_add_mss(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_add_mss_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_query_mss(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_query_mss_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_modify_mss(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_modify_mss_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_del_mss(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_del_mss_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_query_record_policy(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_query_record_policy_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_record_policy_config(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_record_policy_config_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_query_manufacturer(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_query_manufacturer_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_add_modify_manufacturer(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_add_modify_manufacturer_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_del_manufacturer(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_del_manufacturer_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_query_user_own_gu(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_query_user_own_gu_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_add_gu_to_user(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_add_gu_to_user_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_query_user_own_tw(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_query_user_own_tw_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_add_tw_to_user(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_add_tw_to_user_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_query_user_own_tour(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_query_user_own_tour_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_add_tour_to_user(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_add_tour_to_user_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_query_system_time(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_query_system_time_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_set_system_time(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_set_system_time_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_database_backup(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_database_backup_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_database_import(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_database_import_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_bss_add_hd_group(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_bss_add_hd_group_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_bss_add_hd_to_group(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_bss_add_hd_to_group_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_bss_del_hd_from_group(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_bss_del_hd_from_group_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_bss_query_all_hd_group(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_bss_query_all_hd_group_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_bss_query_hd_group_info(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_bss_query_hd_group_info_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_bss_query_all_hd(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_bss_query_all_hd_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_bss_del_hd_group(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_bss_del_hd_group_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_bss_reboot_mss(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_bss_reboot_mss_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_bss_get_hd_format_progress(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_bss_get_hd_format_progress_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_bss_query_gu_record_status(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_bss_query_gu_record_status_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_add_defence_area(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_add_defence_area_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_modify_defence_area(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_modify_defence_area_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_del_defence_area(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_del_defence_area_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_query_defence_area(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_query_defence_area_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_add_defence_map(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_add_defence_map_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_del_defence_map(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_del_defence_map_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_query_defence_map(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_query_defence_map_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_add_defence_gu(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_add_defence_gu_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_modify_defence_gu(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_modify_defence_gu_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_del_defence_gu(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_del_defence_gu_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_query_defence_gu(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_query_defence_gu_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_set_map_href(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_set_map_href_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_set_del_alarm_policy(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_set_del_alarm_policy_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_modify_map_href(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_modify_map_href_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_del_map_href(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_del_map_href_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_query_map_href(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_query_map_href_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_add_tw(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_add_tw_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_modify_tw(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_modify_tw_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_del_tw(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_del_tw_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_query_tw(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_query_tw_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_add_screen(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_add_screen_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_modify_screen(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_modify_screen_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_del_screen(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_del_screen_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_query_screen(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_query_screen_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_query_scr_div(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_query_scr_div_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_add_tour(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_add_tour_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_modify_tour(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_modify_tour_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_del_tour(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_del_tour_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_query_tour(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_query_tour_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_add_tour_step(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_add_tour_step_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_query_tour_step(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_query_tour_step_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_add_group(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_add_group_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_modify_group(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_modify_group_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_del_group(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_del_group_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_query_group(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_query_group_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_add_group_step(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_add_group_step_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_modify_group_step(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_modify_group_step_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_del_group_step(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_del_group_step_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_query_group_step(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_query_group_step_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_config_group_step(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_config_group_step_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_modify_group_step_info(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_modify_group_step_info_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_del_group_step_info(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_del_group_step_info_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_query_group_step_info(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_query_group_step_info_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_query_group_step_div(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_query_group_step_div_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_query_alarm(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_query_alarm_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_del_alarm(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_del_alarm_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_query_del_alarm_policy(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_query_del_alarm_policy_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_bss_platform_upgrade(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_bss_platform_upgrade_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_get_net_interface_config(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_get_net_interface_config_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_get_network_config(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_get_network_config_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_set_network_config(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_set_network_config_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_link_time_policy(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_link_time_policy_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_modify_link_time_policy(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_modify_link_time_policy_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_del_link_time_policy(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_del_link_time_policy_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_query_link_time_policy(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_query_link_time_policy_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_link_record(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_link_record_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_modify_link_record(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_modify_link_record_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_del_link_record(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_del_link_record_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_query_link_record(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_query_link_record_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_link_io(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_link_io_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_modify_link_io(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_modify_link_io_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_del_link_io(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_del_link_io_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_query_link_io(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_query_link_io_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_link_snapshot(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_link_snapshot_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_modify_link_snapshot(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_modify_link_snapshot_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_del_link_snapshot(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_del_link_snapshot_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_query_link_snapshot(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_query_link_snapshot_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_link_preset(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_link_preset_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_modify_link_preset(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_modify_link_preset_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_del_link_preset(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_del_link_preset_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_query_link_preset(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_query_link_preset_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_link_step(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_link_step_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_modify_link_step(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_modify_link_step_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_del_link_step(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_del_link_step_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_query_link_step(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_query_link_step_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_link_map(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_link_map_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_modify_link_map(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_modify_link_map_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_del_link_map(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_del_link_map_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_query_link_map(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_query_link_map_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *jpf_parse_query_server_resource_info(xmlDocPtr doc ,
	xmlNodePtr cur, char *cmd);

int jpf_create_query_server_resource_info_resp(xmlDocPtr doc,
	JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_query_cms_all_ips(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_query_cms_all_ips_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_search_device(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_search_device_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_get_searched_device(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_get_searched_device_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_query_tw_auth_info(xmlDocPtr doc , xmlNodePtr cur, char *cmd);

int
jpf_create_query_tw_auth_info_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_query_alarm_link_auth_info(xmlDocPtr doc , xmlNodePtr cur, char *cmd);

int
jpf_create_query_alarm_link_auth_info_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_auto_add_pu(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_auto_add_pu_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_get_next_puno(xmlDocPtr doc, xmlNodePtr cur, char *cmd);

int
jpf_create_get_next_puno_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_bss_get_initiator_name(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_bss_get_initiator_name_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_bss_set_initiator_name(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_bss_set_initiator_name_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_bss_get_ipsan_info(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_bss_get_ipsan_info_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_bss_add_one_ipsan(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_bss_add_one_ipsan_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_bss_delete_one_ipsan(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_bss_delete_one_ipsan_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_bss_get_one_ipsan_detail(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_bss_get_one_ipsan_detail_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_query_log(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_query_log_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_query_area_dev_online_rate(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_query_area_dev_online_rate_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_validate_gu_map(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_validate_gu_map_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_add_ivs(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_add_ivs_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_query_ivs(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_query_ivs_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_modify_ivs(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_modify_ivs_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_del_ivs(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_del_ivs_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_get_server_flag(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_get_server_flag_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_get_mds_config(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_get_mds_config_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_set_mds_config(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_set_mds_config_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_get_mds_state(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_get_mds_state_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_get_mss_config(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_get_mss_config_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_set_mss_config(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_set_mss_config_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_get_mss_state(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_get_mss_state_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_get_ivs_config(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_get_ivs_config_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_set_ivs_config(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_set_ivs_config_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_get_ivs_state(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_get_ivs_state_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cms_shutdown(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_cms_shutdown_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cms_reboot(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_cms_reboot_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_add_ams(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_add_ams_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_modify_ams(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_modify_ams_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_del_ams(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_del_ams_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_query_ams(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_query_ams_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_query_ams_pu(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_query_ams_pu_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_modify_ams_pu(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_modify_ams_pu_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);


#endif
