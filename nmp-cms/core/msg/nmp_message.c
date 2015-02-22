#include "nmp_message.h"


BEGIN_MSG_ID_MAPPING(cms)
		{"PuRegister" },	/* 0 */
		{"PuHeart" },	/* 1 */
		{"ClientUserLogin" }, /*2*/
		{"CuHeart"}, /*3*/
		{"AdministratorLogin"}, /*4*/
		{"BssKeepAlive"},     /*5*/
		{"SubmitAlarm"},/* 6 */
		{"ChangePuOnlineState"}, /*7*/
		{"ChangeDispatch"},   /*8*/
		{"AddAdministrator"},   /*9*/
		{"ModifyAdministratorPassword"}, /*10*/
		{"DelAdministrator"}, /*11*/
		{"QueryAdministrator"}, /*12*/
		{"ValidateAdministrator"}, /*13*/
		{"AddUserGroup"},   /*14*/
		{"ModifyUserGroup"}, /*15*/
		{"DelUserGroup"}, /*16*/
		{"QueryUserGroup"}, /*17*/
		{"ValidateUserGroup"}, /*18*/
		{"AddUser"},   /*19*/
		{"ModifyUser"}, /*20*/
		{"DelUser"}, /*21*/
		{"QueryUser"}, /*22*/
		{"ValidateUser"}, /*23*/
		{"AddDomain"}, /*24*/
		{"ModifyDomain"}, /*25*/
		{"DelDomain"}, /*26*/
		{"QueryDomain"}, /*27*/
		{"AddOrModifyArea"}, /*28*/
		{"DelArea"}, /*29*/
		{"QueryArea"}, /*30*/
		{"AddPu"},   /*31*/
		{"ModifyPu"}, /*32*/
		{"DelPu"}, /*33*/
		{"QueryPu"}, /*34*/
		{"AddGu"},   /*35*/
		{"ModifyGu"}, /*36*/
		{"DelGu"}, 	/*37*/
		{"QueryGu"}, /*38*/
		{"AddMdu"},   /*39*/
		{"ModifyMdu"}, /*40*/
		{"DelMdu"}, /*41*/
		{"QueryMdu"}, /*42*/
		{"AddOrModifyManufacturer"}, /*43*/
		{"DelManufacturer"}, /*44*/
		{"QueryManufacturer"}, /*45*/
		{"AddGuToUser"}, /*46*/
		{"QueryUserOwnGu"}, /*47*/
		{"AddMduIp"},   /*48*/
		{"DelMduIp"}, /*49*/
		{"QueryMduIp"}, /*50*/
		{"QueryServerTime"}, /*51*/
		{"SetServerTime"}, /*52*/
		{"GetAllArea"}, /*53 */
		{"GetDeviceList"}, /*54*/
		{"GetMediaURL"}, /*55*/
		{"MdsRegister"},	/* 56*/
		{"MdsHeart"}, 	/* 57 */

		{"GetPlatformInfo"}, 	/* 58 */
		{"SetPlatformInfo"}, 	/* 59 */
		{"GetDeviceInfo"}, 	/* 60 */
		{"GetNetworkInfo"},    /* 61 */
		{"SetNetworkInfo"},    /* 62 */
		{"GetEncodeParameter"},  /* 63 */
		{"SetEncodeParameter"},  /* 64 */
		{"GetDisplayParameter"}, /* 65 */
		{"SetDisplayParameter"},/* 66 */
		{"GetRecordParameter"},/* 67 */
		{"SetRecordParameter"},/* 68*/
		{"GetMoveAlarmParameter"},/* 69 */
		{"SetMoveAlarmParameter"},/* 70 */
		{"GetVideoLostAlarmParameter"},/* 71 */
		{"SetVideoLostAlarmParameter"},/* 72 */
		{"GetHideParameter"},/* 73 */
		{"SetHideParameter"},/* 74 */
		{"GetHideAlarmParameter"},/* 75 */
		{"SetHideAlarmParameter"},/* 76*/
		{"GetIoAlarmParameter"},/* 77 */
		{"SetIoAlarmParameter"},/* 78 */
		{"GetSerialParameter"},/* 79 */
		{"SetSerialParameter"},	/*80 */
		{"GetDeviceTime"},/* 81 */
		{"SetDeviceTime"},/* 82 */
		{"GetOSDParameter"},/* 83 */
		{"SetOSDParameter"},/* 84*/
		{"GetJointParameter"}, /*85*/
		{"SetJointParameter"}, /*86*/
		{"GetPTZParameter"}, /*87*/
		{"SetPTZParameter"}, /*88*/
		{"ControlPTZCmd"}, /*89*/
		{"DatabaseBackup"}, /*90*/
		{"ForceUserOffline"}, /*91*/
		{"AddMss"},   /*92*/
		{"ModifyMss"}, /*93*/
		{"DelMss"}, /*94*/
		{"QueryMss"}, /*95*/
		{"QueryRecordPolicy"},  /*96*/
		{"RecordPolicyConfig"},  /*97*/
		{"GetDeviceNTPInfo"}, /* 98 */
		{"SetDeviceNTPInfo"}, /* 99*/
		{"GetPPPOEInfo"}, /* 100 */
		{"SetPPPOEInfo"}, /* 101*/
		{"GetFTPParameter"}, /* 102 */
		{"SetFTPParameter"}, /* 103*/
		{"GetSMTPParameter"}, /* 104 */
		{"SetSMTPParameter"}, /* 105*/
		{"GetUPNPParameter"}, /* 106 */
		{"SetUPNPParameter"}, /* 107*/
		{"GetDiskInfo"}, /* 108 */
		{"FormatDisk"}, /* 109 */
		{"SubmitFormatProgress"}, /* 110 */
		{"FirmwareUpgrade"}, /* 111 */
		{"GetStoreLog"}, /* 112 */
		{"MssRegister"}, /* 113 */
		{"MssHeart"}, /* 114 */
		{"MssGetGuid"}, /*115*/
		{"MssGetRecordPolicy"}, /*116*/
		{"MssGetRoute"}, /*117*/
		{"NotifytRecordPolicyChange"}, /*118*/
		{"MssGetMds"}, /*119*/
		{"MssGetMdsIp"}, /*120*/
		{"DatabaseImport"}, /*121*/
		{"GetAlarm"}, /*122*/
		{"GetAlarmState"}, /*123*/
		{"DealAlarm"}, /*124*/
		{"ControlDevice"}, /*125*/
		{"NotifyModifyDomain"}, /*126*/
		{"AddHdGroup"}, /*127*/
		{"AddHdToGroup"}, /*128*/
		{"DelHdFromGroup"}, /*129*/
		{"QueryAllHdGroup"}, /*130*/
		{"QueryHdGroupInfo"}, /*131*/
		{"QueryAllHd"}, /*132*/
		{"DelHdGroup"}, /*133*/
		{"GetHdFormatProgress"}, /*134*/
		{"GuListChange"}, /*135*/
		{"GetGuMss"}, /* 136 */
		{"GetMssStoreLog"}, /* 137 */
		{"AddDefenceArea"}, /* 138 */
		{"ModifyDefenceArea"}, /* 139 */
		{"QueryAllDefence"}, /* 140 */
		{"DelDefenceArea"}, /* 141 */
		{"AddDefenceMap"}, /* 142 */
		{"QueryDefenceMap"}, /* 143 */
		{"DelDefenceMap"}, /* 144 */
		{"AddDefenceGu"}, /* 145 */
		{"ModifyDefenceGu"}, /* 146 */
		{"QueryDefenceGu"}, /* 147 */
		{"DelDefenceGu"}, /* 148 */
		{"SetMapHref"}, /* 149 */
		{"ModifyMapHref"}, /* 150 */
		{"QueryMapHref"}, /* 151 */
		{"DelMapHref"}, /* 152 */
		{"NotifyModifyMss"}, /* 153 */
		{"GetDefenceArea"}, /* 154 */
		{"GetDefenceMap"}, /* 155 */
		{"GetDefenceGu"}, /* 156 */
		{"GetMapHref"}, /* 157 */
		{"GetMdsInfo"}, /* 158 */
		{"PlatformUpgrade"}, /* 159 */
		{"GetTransparentParam"}, /* 160 */
		{"SetTransparentParam"}, /* 161 */
		{"GetDdnsConfig"}, /* 162 */
		{"SetDdnsConfig"}, /* 163 */
		{"QueryAlarm"}, /* 164 */
		{"DelAlarm"}, /* 165 */
		{"QueryAutoDelAlarmPolicy"}, /* 166 */
		{"SetAutoDelAlarmPolicy"}, /* 167 */
		{"AddTw"},   /*168*/
		{"ModifyTw"}, /*169*/
		{"DelTw"}, /*170*/
		{"QueryTw"}, /*171*/
		{"AddScreen"},   /*172*/
		{"ModifyScreen"}, /*173*/
		{"DelScreen"}, /*174*/
		{"QueryScreen"}, /*175*/
		{"QueryScreenDivision"}, /*176*/
		{"GetGuMapLocation"}, /*177*/
		{"GetTw"}, /*178*/
		{"GetScreens"}, /*179*/
		{"GetScreenDivision"}, /*180*/
		{"GetScreenState"}, /*181*/
		{"ChangeDivisionMode"}, /*182*/
		{"TwPlayNotify"},/*183*/
		{"FullScreen"},/*184*/
		{"ExitFullScreen"},/*185*/
		{"NotifyMessage"}, /*186*/
		{"AddTour"},   /*187*/
		{"ModifyTour"}, /*188*/
		{"DelTour"}, /*189*/
		{"QueryTour"}, /*190*/
		{"AddTourStep"},   /*191*/
		{"QueryTourStep"}, /*192*/
		{"TwPlay"}, /*193*/
		{"TwRunStep"}, /*194*/
		{"TwRunTour"}, /*195*/
		{"TwRunGroup"}, /*196*/
		{"TwStopTour"}, /*197*/
		{"TwStopGroup"}, /*198*/
		{"TwTourRunState"}, /*199*/
		{"TwGroupRunState"}, /*200*/
		{"TwStopDivision"}, /*201*/
		{"QueryDivisionMode"}, /*202*/
		{"TwOperateNotify"},/*203*/
		{"ClearDivision"},/*204*/
		{"decoderOnlineStateNotify"},/*205*/
		{"GetTour"},/*206*/
		{"GetTourStep"},/*207*/
		{"AddGroup"},   /*208*/
		{"ModifyGroup"}, /*209*/
		{"DelGroup"}, /*210*/
		{"QueryGroup"}, /*211*/
		{"AddGroupStep"},   /*212*/
		{"ModifyGroupStep"}, /*213*/
		{"DelGroupStep"}, /*214*/
		{"QueryGroupStep"}, /*215*/
		{"ConfigGroupStep"},   /*216*/
		{"ModifyGroupStepInfo"}, /*217*/
		{"DelGroupStepInfo"}, /*218*/
		{"QueryGroupStepInfo"}, /*219*/
		{"GetGroup"},/*220*/
		{"GetNetInterfaceConfig"},     /*221*/
 		{"GetNetworkConfig"},     /*222*/
		{"SetNetworkConfig"}, /*223*/
		{"GetPresetPointSet"}, /* 224 */
		{"SetPresetPoint"}, /* 225*/
		{"GetCruiseWaySet"}, /* 226 */
		{"GetCruiseWay"}, /* 227 */
		{"AddCruiseWay"}, /* 228*/
		{"ModifyCruiseWay"}, /* 229 */
		{"SetCruiseWay"}, /* 230*/
		{"3DControl"},     /*231*/
		{"AmsRegister"}, /* 232 */
		{"AmsHeart"}, /* 233 */
		{"LinkTimePolicyConfig"}, /* 234 */
		{"ModifyLinkTimePolicy"}, /* 235 */
		{"QueryLinkTimePolicy"}, /* 236 */
		{"DelLinkTimePolicy"}, /* 237 */
		{"LinkRecordConfig"}, /* 238 */
		{"ModifyLinkRecord"}, /* 239 */
		{"QueryLinkRecord"}, /* 240 */
		{"DelLinkRecord"}, /* 241 */
		{"AlarmLinkRecord"},    /*242*/
		{"QueryServerResourceInfo"}, /*243*/
		{"GetLicenseInfo"}, /*244*/
		{"LinkStepConfig"}, /* 245 */
		{"ModifyLinkStep"}, /* 246 */
		{"QueryLinkStep"}, /* 247 */
		{"DelLinkStep"}, /* 248 */
		{"AlarmLinkStep"},    /*249*/
		{"SearchPu"},    /*250*/
		{"GetSearchedPus"},    /*251*/
		{"QueryCmsAllIp"},    /*252*/
		{"AutoAddPu"},    /*253*/
		{"GetNextPuNo"},    /*254*/
		{"GetInitiatorName"},    /*255*/
		{"SetInitiatorName"},    /*256*/
		{"GetIpsanInfo"},    /*257*/
		{"AddOneIpsan"},    /*258*/
		{"DeleteOneIpsan"},    /*259*/
		{"GetOneIpsanDetail"},    /*260*/
		{"QueryTwAuthInfo"},    /*261*/
		{"GetTwLicenseInfo"},    /*262*/
		{"QueryGuRecordStatus"},    /*263*/
		{"GetAreaDevice"},    /*264*/
		{"GetAreaInfo"},    /*265*/
		{"LinkIOConfig"}, /* 266 */
		{"ModifyLinkIO"}, /* 267 */
		{"QueryLinkIO"}, /* 268 */
		{"DelLinkIO"}, /* 269 */
		{"AlarmLinkIO"},    /*270*/
		{"LinkSnapshotConfig"}, /* 271 */
		{"ModifyLinkSnapshot"}, /* 272 */
		{"QueryLinkSnapshot"}, /* 273 */
		{"DelLinkSnapshot"}, /* 274 */
		{"AlarmLinkSnapshot"},    /*275*/
		{"RebootMss"}, /* 276 */
		{"GetResolutionInfo"}, 	/* 277 */
		{"SetResolutionInfo"}, 	/* 278*/
		{"GetIrcutControlInfo"}, 	/* 279 */
		{"SetIrcutControlInfo"}, 	/* 280 */
		{"LinkPresetConfig"}, /* 281 */
		{"ModifyLinkPreset"}, /* 282 */
		{"QueryLinkPreset"}, /* 283 */
		{"DelLinkPreset"}, /* 284 */
		{"AlarmLinkPreset"},    /*285*/
		{"3DGoback"},    /* 286 */
		{"LinkTourConfig"}, /* 287 */
		{"ModifyLinkTour"}, /* 288 */
		{"QueryLinkTour"}, /* 289 */
		{"DelLinkTour"}, /* 290 */
		{"AlarmLinkTour"},    /*291*/
		{"LinkGroupConfig"}, /* 292 */
		{"ModifyLinkGroup"}, /* 293 */
		{"QueryLinkGroup"}, /* 294 */
		{"DelLinkGroup"}, /* 295 */
		{"AlarmLinkGroup"},    /*296*/
		{"QueryLog"},    /*297*/
		{"QueryAlarmLinkAuthInfo"},    /*298*/
		{"AddTwToUser"}, /*299*/
		{"QueryUserOwnTw"}, /*300*/
		{"AddTourToUser"}, /*301*/
		{"QueryUserOwnTour"}, /*302*/
		{"QueryGroupStepDiv"}, /*303*/
		{"QueryAreaDevOnlineRate"}, /*304*/
		{"LinkMapConfig"}, /* 305 */
		{"ModifyLinkMap"}, /* 306 */
		{"QueryLinkMap"}, /* 307 */
		{"DelLinkMap"}, /* 308 */
		{"ValidateGuMap"}, /* 309 */
		{"AlarmLinkMap"}, /* 310 */
		{"CuModifyUserPwd"}, /*311*/
		{"AddIvs"},   /*312*/
		{"ModifyIvs"}, /*313*/
		{"DelIvs"}, /*314*/
		{"QueryIvs"}, /*315*/
		{"IvsRegister"}, /* 316 */
		{"IvsHeart"}, /* 317 */
		{"GetServerFlag"}, /* 318 */
		{"GetMdsConfig"}, /* 319 */
		{"SetMdsConfig"}, /* 320 */
		{"GetMdsState"}, /* 321 */
		{"GetMssConfig"}, /* 322 */
		{"SetMssConfig"}, /* 323 */
		{"GetMssState"}, /* 324 */
		{"GetIvsConfig"}, /* 325 */
		{"SetIvsConfig"}, /* 326 */
		{"GetIvsState"}, /* 327 */
		{"CmsShutdown"}, /* 328 */
		{"CmsReboot"}, /* 329 */
		{"AddAms"}, /* 330 */
		{"ModifyAms"}, /* 331 */
		{"DelAms"}, /* 332 */
		{"QueryAms"}, /* 333 */
		{"QueryAmsPu"}, /* 334 */
		{"ModifyAmsPu"}, /* 335 */
		{"AmsDeviceInfoChange"}, /* 336 */
		{"AmsGetDeviceInfo"}, /* 337 */
		{"GetDefDisplayParameter"}, /* 338 */
		{"QueryGuid"}, /* 339 */
		{"QueryScreenID"}, /* 340 */
		{"QueryUserGuids"}, /* 341 */
		{"SetUserGuids"}, /* 342 */
		{"SetScreenNum"}, /* 343 */
		{"QueryTourID"}, /* 344 */
		{"SetTourNum"}, /* 345 */
		{"QueryGroupID"}, /* 346 */
		{"SetGroupNum"}, /* 347 */

END_MSG_ID_MAPPING(cms)

//:~ End
