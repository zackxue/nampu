/***************************************************
 * JXJ Platform DB
 * Copyright (c) szjxj 2011
 *
 * -------------------------------------------------
 * 2011-05-10   fangyi  create
 * 2012-02-29   add mss table
 * 2012-10-26   add tour
 * 2012-11-29   add group
****************************************************/

/*
 * Create new database: "jxj_platform_db"
 * Drop it if exists.
*/
DROP DATABASE IF EXISTS jxj_platform_db;

CREATE DATABASE jxj_platform_db
    DEFAULT CHARACTER SET utf8
    COLLATE utf8_general_ci;

USE jxj_platform_db;

/*
 *  Section 1, Tables for domain management.
 *  Including:
 *          domain_table
 *          domain_area_table
 *          super_user_table
*/


/*
 * Table No.1-01
 *
 * Domain table.
*/
CREATE TABLE domain_table(
    `dm_id` CHAR(8) NOT NULL                            COMMENT '域ID',
    `dm_name` VARCHAR(64) NOT NULL                      COMMENT '域名称',
    `dm_ip` VARCHAR(64) NOT NULL                        COMMENT '域IP地址或域名',
    `dm_port` INT(4) NOT NULL                           COMMENT '域基准端口',
    `dm_type` INT(4) NOT NULL                           COMMENT '域类型：0-本域；1-子域；2-父域',

    PRIMARY KEY(`dm_id`)
)ENGINE=InnoDB;

INSERT INTO domain_table(`dm_id`, `dm_name`, `dm_type`) VALUES('DM-00001', '监控中心-未命名', 0);

/*
 * Table No.1-02
 *
 * Domain area table.
*/
CREATE TABLE domain_area_table(
    `area_id` INT(4) NOT NULL AUTO_INCREMENT            COMMENT '机构ID',
    `area_name` VARCHAR(64) NOT NULL                    COMMENT '机构名',
    `area_parent` INT(4)                                COMMENT '父机构ID',
    `user_name` VARCHAR(64) DEFAULT NULL                COMMENT '区域用户的名字', 
    `user_phone` VARCHAR(64) DEFAULT NULL               COMMENT '区域用户的电话', 
    `user_address` VARCHAR(256) DEFAULT NULL            COMMENT '区域用户的单位地址',
    `description`  VARCHAR(256) DEFAULT NULL            COMMENT '区域描述',

    PRIMARY KEY(`area_id`),
    FOREIGN KEY(`area_parent`) REFERENCES domain_area_table(`area_id`)
        ON DELETE CASCADE
)ENGINE=InnoDB;

INSERT INTO domain_area_table(`area_name`) VALUES('根区域');

/*
 * Table No.1-03
 *
 * Super user table.
*/
CREATE TABLE super_user_table(
    `su_name` VARCHAR(64) BINARY NOT NULL               COMMENT '管理员名字',
    `su_password` VARCHAR(64) BINARY NOT NULL           COMMENT '管理员密码',
    `su_type` INT(4) NOT NULL UNIQUE AUTO_INCREMENT     COMMENT '管理员类别',

    PRIMARY KEY(`su_name`)
)ENGINE=InnoDB;

INSERT INTO super_user_table VALUES('admin', 'Admin', 0);


/*
 *  Section 2, Tables for device management.
 *  Including:
 *          dev_manufacturer_table
 *          dev_configure_table
 *          dev_guconf_table
 *          dev_running_info_table
*/

/*
 * Table No.2-01
 *
 * Device manufacturers table.
*/
CREATE TABLE dev_manufacturer_table(
    `mf_id` CHAR(3) NOT NULL                            COMMENT '厂商ID',
    `mf_name` VARCHAR(64) NOT NULL UNIQUE               COMMENT '厂商名',

    PRIMARY KEY(`mf_id`)
)ENGINE=InnoDB; 

INSERT INTO dev_manufacturer_table VALUES('HIK', '杭州海康威视');
INSERT INTO dev_manufacturer_table VALUES('DAH', '浙江大华');
INSERT INTO dev_manufacturer_table VALUES('JXJ', '深圳市佳信捷');
INSERT INTO dev_manufacturer_table VALUES('ENC', '智能高清网络设备供应商');
INSERT INTO dev_manufacturer_table VALUES('HBN', '汉邦高科');
INSERT INTO dev_manufacturer_table VALUES('HNW', '霍尼韦尔');
INSERT INTO dev_manufacturer_table VALUES('XMT', '杭州雄迈');
INSERT INTO dev_manufacturer_table VALUES('TPS', '深圳天视通');


/*
 * Table No.2-02
 *
 * Device configuration table.
*/
CREATE TABLE dev_configure_table(
    `pu_id` CHAR(16) NOT NULL                           COMMENT '设备ID号',
    `pu_domain` CHAR(8) NOT NULL                        COMMENT '设备所属的域',
    `pu_info` VARCHAR(64) NOT NULL DEFAULT ''           COMMENT '设备备注信息',
    `pu_keep_alive_freq` INT(4) NOT NULL                COMMENT '设备心跳周期',
    `pu_type` INT(4) NOT NULL                           COMMENT '设备类型：DVS,DVR,IPC,',
    `pu_minor_type` INT(4) NOT NULL DEFAULT 0           COMMENT '设备子类型：DVS,DVR,IPC,IPNC,软解，硬解',
    `pu_manufacturer` CHAR(3) NOT NULL                  COMMENT '设备厂商ID',
    `pu_mdu` CHAR(16)                                   COMMENT '设备所属转发服务器',
    `pu_area` INT(4) NOT NULL                           COMMENT '设备所属的机构',

    PRIMARY KEY(`pu_id`, `pu_domain`),
    FOREIGN KEY(`pu_domain`) REFERENCES domain_table(`dm_id`)
        ON UPDATE CASCADE ON DELETE CASCADE,
    FOREIGN KEY(`pu_manufacturer`) REFERENCES dev_manufacturer_table(`mf_id`)
        ON DELETE CASCADE,
    FOREIGN KEY(`pu_area`) REFERENCES domain_area_table(`area_id`)
        ON DELETE CASCADE
)ENGINE=InnoDB;


/*
 * Table No.2-03
 *
 * Device gu configuration table.
*/
CREATE TABLE dev_guconf_table(
    `gu_id` CHAR(24) NOT NULL                           COMMENT '业务点ID',
    `gu_domain` CHAR(8) NOT NULL                        COMMENT '业务点所属的域',
    `gu_puid` CHAR(16) NOT NULL                         COMMENT '业务点所属的PU',
    `gu_name` VARCHAR(64) NOT NULL                      COMMENT '业务点名称',
    `gu_type` INT(4) NOT NULL                           COMMENT '业务点类型：音视频;告警;显示,',
    `gu_attributes` INT(4) NOT NULL                     COMMENT '业务点属性',
    `ivs_id` CHAR(16)                                   COMMENT '智能分析服务器',

    PRIMARY KEY(`gu_id`, `gu_domain`),
    INDEX(`gu_puid`, `gu_domain`),
    FOREIGN KEY(`gu_puid`, `gu_domain`) REFERENCES dev_configure_table(`pu_id`, `pu_domain`)
        ON UPDATE CASCADE ON DELETE CASCADE
)ENGINE=InnoDB;


/*
 * Table No.2-04
 *
 * Device running info table.
*/
CREATE TABLE dev_running_info_table(
    `pu_id` CHAR(16) NOT NULL                           COMMENT '设备PUID',
    `pu_domain` CHAR(8) NOT NULL                        COMMENT '设备所属域',
    `pu_last_alive` VARCHAR(32)                         COMMENT '设备上一次活动时间',
    `pu_last_ip` VARCHAR(128)                           COMMENT '设备上一次注册IP',
    `pu_state` INT(4) NOT NULL                          COMMENT 'PU在线状态',
    `pu_last_cmsip` VARCHAR(128)                        COMMENT '设备上一次注册IP',
    `pu_registered` INT(4) NOT NULL                     COMMENT '设备是否注册，还是已经下线。防止心跳将下线的设备再次设置为在线状态（线程池无法保证任务按投递的顺序执行）',

    PRIMARY KEY(`pu_id`, `pu_domain`),
    FOREIGN KEY(`pu_id`, `pu_domain`) REFERENCES dev_configure_table(`pu_id`, `pu_domain`)
        ON UPDATE CASCADE ON DELETE CASCADE    
)ENGINE=InnoDB;


/*
 * Table No.3-01
 *
 * User group table.
*/
CREATE TABLE user_group_table(
    `group_id` INT(4) NOT NULL AUTO_INCREMENT           COMMENT '用户组ID',
    `group_permissions` INT(4) NOT NULL                 COMMENT '用户组权限',
    `group_rank` INT(4) NOT NULL                        COMMENT '用户组等级',
    `group_name` VARCHAR(64) NOT NULL UNIQUE            COMMENT '用户组名称',

    PRIMARY KEY(`group_id`)
)ENGINE=InnoDB;


/*
 * Table No.3-02
 *
 * User management table.
*/
CREATE TABLE user_manage_table(
    `user_name` VARCHAR(64) BINARY NOT NULL             COMMENT '用户名',
    `user_group` INT(4) NOT NULL                        COMMENT '用户所属的组',
    `user_sex` INT(4) NOT NULL                          COMMENT '用户性别',
    `user_password` VARCHAR(64) BINARY NOT NULL         COMMENT '用户密码',
    `user_phone_number` VARCHAR(16)                     COMMENT '用户电话',
    `user_description` VARCHAR(64)                      COMMENT '备注信息',
    `user_no` INT(4) NOT NULL UNIQUE AUTO_INCREMENT     COMMENT '用户名编号，查询时排序',

    PRIMARY KEY(`user_name`),
    FOREIGN KEY(`user_group`) REFERENCES user_group_table(`group_id`)
        ON DELETE CASCADE
)ENGINE=InnoDB;


/*
 * Table No.3-03
 *
 * User gu configuratoin table.
*/
CREATE TABLE user_guconf_table(
    `user_name` VARCHAR(64) BINARY NOT NULL             COMMENT '用户名',
    `user_guid` CHAR(24) NOT NULL                       COMMENT '用户所属业务点',
    `user_guid_domain` CHAR(8) NOT NULL                 COMMENT '该业务点对应的域',

    PRIMARY KEY(`user_name`, `user_guid`, `user_guid_domain`),
    INDEX(`user_guid`, `user_guid_domain`),
    FOREIGN KEY(`user_guid`, `user_guid_domain`) REFERENCES dev_guconf_table(`gu_id`, `gu_domain`)
        ON UPDATE CASCADE ON DELETE CASCADE,
    FOREIGN KEY(`user_name`) REFERENCES user_manage_table(`user_name`)
        ON DELETE CASCADE
)ENGINE=InnoDB;


/*
 * Table No.4-01
 *
 * Dispatch unit table.
*/
CREATE TABLE dispatch_unit_table(
    `mdu_id` CHAR(16) NOT NULL                          COMMENT '转发服务器ID',
    `mdu_name` VARCHAR(64) NOT NULL UNIQUE              COMMENT '转发服务器名称',
    `mdu_type` INT(4) NOT NULL                          COMMENT '转发服务器类型',
    `mdu_keep_alive_freq` INT(4) NOT NULL               COMMENT '心跳周期',
    `mdu_pu_port` INT(4) NOT NULL                       COMMENT '转发服务器设备端口号',
    `mdu_rtsp_port` INT(4) NOT NULL                     COMMENT '转发服务器流媒体端口号',
    `mdu_state` INT(4) NOT NULL                         COMMENT '转发服务器在线状态',
    `auto_get_ip_enable` INT(4) NOT NULL DEFAULT 1      COMMENT '使能获取转发服务ip地址', 

    PRIMARY KEY(`mdu_id`)
)ENGINE=InnoDB;

INSERT INTO dispatch_unit_table VALUES('MDS-V-001', '本地转发', 0,30,10000,554,0,1);

/*
 * Table No.4-02
 *
 * Dispatch unit IP address table.
*/
CREATE TABLE dispatch_unit_ip_table(
    `mdu_id` CHAR(16) NOT NULL                          COMMENT '转发服务器ID',
    `mdu_cmsip` VARCHAR(128) NOT NULL                   COMMENT '用户连接的CMS IP或域名',
    `mdu_ip` VARCHAR(128) NOT NULL                      COMMENT '对应的转发服务器IP或域名',

    PRIMARY KEY(`mdu_id`, `mdu_cmsip`) ,
    FOREIGN KEY(`mdu_id`) REFERENCES dispatch_unit_table(`mdu_id`)
        ON DELETE CASCADE
)ENGINE=InnoDB;


/*
 * Table No.4-03
 *
 * Dispatch unit status table.
*/
CREATE TABLE dispatch_unit_status_table(
    `mdu_id` CHAR(16) NOT NULL                          COMMENT '转发服务器ID',
    `mdu_report_time` VARCHAR(8) NOT NULL               COMMENT '本状态的记录时间',
    `mdu_pu_counts` INT(4) NOT NULL                     COMMENT '转发的设备数',
    `mdu_stream_in` INT(4) NOT NULL                     COMMENT '转发的入码流路数',
    `mdu_stream_out` INT(4) NOT NULL                    COMMENT '转发的出码流路数',
    `mdu_bitrate_in` INT(4) NOT NULL                    COMMENT '转发的入码流大小',
    `mdu_bitrate_out` INT(4) NOT NULL                   COMMENT '转发的出码流大小',
    `mdu_cpu_urate` INT(4) NOT NULL                     COMMENT '转发的CPU使用百分比',

    PRIMARY KEY(`mdu_id`, `mdu_report_time`)
)ENGINE=InnoDB;


/*
 * Table No.5-01
 *
 * store unit table.
*/
CREATE TABLE store_unit_table(
    `mss_id` CHAR(16) NOT NULL                          COMMENT '存储服务器ID',
    `mss_name` VARCHAR(64) NOT NULL UNIQUE              COMMENT '存储服务器名称',
    `mss_keep_alive_freq` INT(4) NOT NULL               COMMENT '心跳周期',
    `mss_state` INT(4) NOT NULL                         COMMENT '存储服务器在线状态',
    `mss_storage_type` INT(4) NOT NULL                  COMMENT '磁盘类型，0：磁盘阵列，1：本地硬盘',
    `mss_mode` INT(4) NOT NULL                          COMMENT '运行模式，0：录像模式，1：配置模式',
    `mss_last_ip` VARCHAR(128)                          COMMENT '存储服务器上一次注册IP',

    PRIMARY KEY(`mss_id`)
)ENGINE=InnoDB; 

INSERT INTO store_unit_table VALUES('MSS-0001', '本地存储',30,0,1,1,NULL);

/*
 * Table No.5-02
 *
 * record policy table.
*/
CREATE TABLE record_policy_table(
    `gu_id` CHAR(24) NOT NULL                           COMMENT '录像的业务点',
    `guid_domain` CHAR(8) NOT NULL                      COMMENT '该业务点对应的域',
    `mss_id` CHAR(16) NOT NULL                          COMMENT '存储服务器ID',
    `level` INT(4) NOT NULL DEFAULT 0                   COMMENT '码流类型',
    `mss_policy` VARCHAR(20480)                         COMMENT '业务ID所对应的录像策略',
    `hd_group_id1` VARCHAR(16)                          COMMENT '硬盘组1',
    `hd_group_id2` VARCHAR(16)                          COMMENT '硬盘组2',
    `hd_group_id3` VARCHAR(16)                          COMMENT '硬盘组3',
    `hd_group_id4` VARCHAR(16)                          COMMENT '硬盘组4',
    `hd_group_id5` VARCHAR(16)                          COMMENT '硬盘组5',  

    PRIMARY KEY(`mss_id`, `gu_id`, `guid_domain`),
    FOREIGN KEY(`gu_id`, `guid_domain`) REFERENCES dev_guconf_table(`gu_id`, `gu_domain`)
        ON UPDATE CASCADE ON DELETE CASCADE,
    FOREIGN KEY(`mss_id`) REFERENCES store_unit_table(`mss_id`)
        ON DELETE CASCADE    
)ENGINE=InnoDB; 


/*
 * Table No.5-03
 *
 * hard disk group table.
*/
CREATE TABLE hd_group_table(
    `mss_id` CHAR(16) NOT NULL                          COMMENT '存储服务器ID',   
    `mss_policy` VARCHAR(20480)                         COMMENT '业务ID所对应的录像策略',
    `hd_group_id` VARCHAR(16)                           COMMENT '硬盘组1',
    `hd_group_name` VARCHAR(64)  NOT NULL               COMMENT '硬盘组2',

    PRIMARY KEY(`mss_id`, `hd_group_id`),
    FOREIGN KEY(`mss_id`) REFERENCES store_unit_table(`mss_id`)
        ON DELETE CASCADE    
)ENGINE=InnoDB; 


/*
 * Table No.6-01
 *
 * alarm information table.
*/
CREATE TABLE alarm_info_table(
    `alarm_id` INT(4) NOT NULL                          COMMENT '告警ID',   
    `gu_domain` CHAR(8) NOT NULL                        COMMENT '业务ID所属的域',
    `gu_id` CHAR(24) NOT NULL                           COMMENT '业务ID',
    `pu_id` CHAR(16) NOT NULL                           COMMENT '设备ID',
    `pu_name` VARCHAR(64) DEFAULT 'UNKNOWN'             COMMENT '设备名',
    `gu_name` VARCHAR(64) DEFAULT 'UNKNOWN'             COMMENT '业务名',
    `alarm_type` INT(4) NOT NULL                        COMMENT '告警类型',
    `alarm_time` CHAR(20) NOT NULL                      COMMENT '告警时间',
    `alarm_info` VARCHAR(128)                           COMMENT '告警内容',
    `state` INT(4) NOT NULL DEFAULT 1                   COMMENT '告警处理状态',       
    `operator` VARCHAR(64)                              COMMENT '处理告警的操作人员',
    `deal_time` CHAR(20)                                COMMENT '处理告警时间',    
    `description` VARCHAR(256)                          COMMENT '备注信息',    
    `submit_time` CHAR(20)                              COMMENT '设备告警时间', 

    PRIMARY KEY(`alarm_id`) 
)ENGINE=InnoDB; 


/*
 * Table No.7-01
 *
 * defence area table.
*/
CREATE TABLE defence_area_table(
    `defence_area_id` INT(4) NOT NULL                   COMMENT '防区ID',
    `defence_enable` INT(4) NOT NULL                    COMMENT '设防使能',    
    `policy` VARCHAR(256)                               COMMENT '设防策略', 

    PRIMARY KEY(`defence_area_id`),
    FOREIGN KEY(`defence_area_id`) REFERENCES domain_area_table(`area_id`)
        ON DELETE CASCADE  
)ENGINE=InnoDB; 


/*
 * Table No.7-02
 *
 * map table.
*/
CREATE TABLE defence_map_table(
    `map_id` INT(4) NOT NULL AUTO_INCREMENT            COMMENT '地图ID',
    `defence_area_id` INT(4) NOT NULL                  COMMENT '防区ID',
    `map_name` VARCHAR(64) NOT NULL                    COMMENT '地图名称',    
    `map_location` VARCHAR(64) NOT NULL                COMMENT '地图定位', 

    PRIMARY KEY(`map_id`),
    FOREIGN KEY(`defence_area_id`) REFERENCES defence_area_table(`defence_area_id`)
        ON DELETE CASCADE
)ENGINE=InnoDB; 



/*
 * Table No.7-03
 *
 * map gu table.
*/
CREATE TABLE map_gu_table(
    `map_id` INT(4) NOT NULL                                COMMENT '地图ID',
    `gu_domain` CHAR(8) NOT NULL                            COMMENT '业务点所属的域',    
    `gu_id` VARCHAR(24)                                     COMMENT '业务点ID', 
    `coordinate_x` DOUBLE(8,4) NOT NULL                     COMMENT '业务点在地图上X轴的相对位置',
    `coordinate_y` DOUBLE(8,4) NOT NULL                     COMMENT '业务点在地图上Y轴的相对位置',

    PRIMARY KEY(`map_id`, `gu_domain`, `gu_id`),
    FOREIGN KEY(`map_id`) REFERENCES defence_map_table(`map_id`)
        ON DELETE CASCADE,
    FOREIGN KEY(`gu_id`, `gu_domain`) REFERENCES dev_guconf_table(`gu_id`, `gu_domain`)
        ON UPDATE CASCADE ON DELETE CASCADE    
)ENGINE=InnoDB; 


/*
 * Table No.7-04
 *
 * map herf table.
*/
CREATE TABLE map_href_table(
    `src_map_id` INT(4) NOT NULL                         COMMENT '源地图ID',
    `dst_map_id` CHAR(8) NOT NULL                        COMMENT '目标地图ID',    
    `coordinate_x` DOUBLE(8,4) NOT NULL                  COMMENT '业务点在地图上X轴的相对位置',
    `coordinate_y` DOUBLE(8,4) NOT NULL                  COMMENT '业务点在地图上Y轴的相对位置',

    PRIMARY KEY(`src_map_id`, `dst_map_id`),
    FOREIGN KEY(`src_map_id`) REFERENCES defence_map_table(`map_id`)
        ON DELETE CASCADE

)ENGINE=InnoDB; 


/*
 * Table No.8-01
 *
 * parameter config table.
*/
CREATE TABLE param_config_table(
    `id` INT(4) NOT NULL                                COMMENT '参数ID',
    `value` VARCHAR(4096)                               COMMENT '参数值', 

    PRIMARY KEY(`id`)
)ENGINE=InnoDB; 

INSERT INTO param_config_table VALUES(1, '100,10100');

/*
 * Table No.9-01
 *
 * tv wall table.
*/
CREATE TABLE tw_table(
    `tw_id` INT(4) NOT NULL  AUTO_INCREMENT             COMMENT '电视墙ID',
    `tw_name` VARCHAR(64) NOT NULL UNIQUE               COMMENT '电视墙名称', 
    `line_num` INT(4) NOT NULL  DEFAULT 3               COMMENT '电视墙分割的行数', 
    `column_num` INT(4) NOT NULL  DEFAULT 3             COMMENT '电视墙分割的列数', 

    PRIMARY KEY(`tw_id`)
)ENGINE=InnoDB; 


/*
 * Table No.9-02
 *
 * tw screens table.
*/
CREATE TABLE tw_screens_table(
    `scr_id` INT(4) NOT NULL AUTO_INCREMENT              COMMENT '屏幕ID',
    `tw_id` INT(4) NOT NULL                              COMMENT '电视墙ID',
    `dis_domain` CHAR(8) NOT NULL                        COMMENT '显示通道所属的域',    
    `dis_guid` VARCHAR(24) NOT NULL                      COMMENT '显示通道', 
    `coordinate_x` DOUBLE(8,4) NOT NULL                  COMMENT '屏幕在电视墙上X轴的相对位置',
    `coordinate_y` DOUBLE(8,4) NOT NULL                  COMMENT '屏幕在电视墙上Y轴的相对位置', 
    `length` DOUBLE(8,4) NOT NULL                        COMMENT '屏幕的长',       
    `width` DOUBLE(8,4) NOT NULL                         COMMENT '屏幕的宽',

    PRIMARY KEY(`scr_id`),
    CONSTRAINT uc_guid UNIQUE (dis_domain,dis_guid),
    FOREIGN KEY(`tw_id`) REFERENCES tw_table(`tw_id`)
        ON DELETE CASCADE,
    FOREIGN KEY(`dis_guid`, `dis_domain`) REFERENCES dev_guconf_table(`gu_id`, `gu_domain`)
        ON UPDATE CASCADE ON DELETE CASCADE    
)ENGINE=InnoDB; 


/*
 * Table No.9-03
 *
 * screens division table.
*/
CREATE TABLE tw_scr_division_table(
    `div_id` INT(4) NOT NULL AUTO_INCREMENT              COMMENT '分屏模式ID',
    `div_name` VARCHAR(64)                               COMMENT '分屏模式', 
    `description` VARCHAR(256)                           COMMENT '描述', 

    PRIMARY KEY(`div_id`)
)ENGINE=InnoDB; 

INSERT INTO tw_scr_division_table VALUES (1, '一分屏',NULL),(2, '四分屏',NULL),(3, '六分屏',NULL),(4, '九分屏',NULL),(5, '十六分屏',NULL),(6, '二十五分屏',NULL),(7, '三十六分屏',NULL),(8, '同一设备拼接',NULL),(9, '保留',NULL),(10, '保留',NULL);

/*
 * Table No.9-04
 *
 * User tw configuratoin table.
*/
CREATE TABLE user_twconf_table(
    `user_name` VARCHAR(64) BINARY NOT NULL             COMMENT '用户名',
    `user_tw_id` INT(4) NOT NULL                        COMMENT '电视墙ID',

    PRIMARY KEY(`user_name`, `user_tw_id`),
    FOREIGN KEY(`user_tw_id`) REFERENCES tw_table(`tw_id`)
        ON UPDATE CASCADE ON DELETE CASCADE,
    FOREIGN KEY(`user_name`) REFERENCES user_manage_table(`user_name`)
        ON DELETE CASCADE
)ENGINE=InnoDB;  

/*
 * Table No.10-01
 *
 * tour table.
*/
CREATE TABLE tour_table(
    `tour_id` INT(4) NOT NULL  AUTO_INCREMENT           COMMENT '巡回ID',
    `tour_name` VARCHAR(64) NOT NULL UNIQUE             COMMENT '巡回名称', 
    `auto_jump` INT(4) NOT NULL  DEFAULT 0              COMMENT '返回步无效时工作模式，0：停留，1：跳下一步', 

    PRIMARY KEY(`tour_id`)
)ENGINE=InnoDB;

/*
 * Table No.10-02
 *
 * tour step table.
*/
CREATE TABLE tour_step_table(
    `tour_id` INT(4) NOT NULL                           COMMENT '巡回ID',
    `step_no` INT(4) NOT NULL                           COMMENT '巡回步号',
    `interval` INT(4) NOT NULL                          COMMENT '巡回步驻留时间',
    `encoder_domain` CHAR(8) NOT NULL                   COMMENT '巡回步对应的编码器所在的域',
    `encoder_guid` CHAR(24) NOT NULL                    COMMENT '巡回步对应的编码器ID',
    `level` INT(4) NOT NULL DEFAULT 0                   COMMENT '码流类型',

    PRIMARY KEY(`tour_id`, `step_no`),
    FOREIGN KEY(`tour_id`) REFERENCES tour_table(`tour_id`)
        ON DELETE CASCADE,
    FOREIGN KEY(`encoder_guid`, `encoder_domain`) REFERENCES dev_guconf_table(`gu_id`, `gu_domain`)
        ON UPDATE CASCADE ON DELETE CASCADE
)ENGINE=InnoDB;

/*
 * Table No.10-03
 *
 * User tour configuratoin table.
*/
CREATE TABLE user_tourconf_table(
    `user_name` VARCHAR(64) BINARY NOT NULL             COMMENT '用户名',
    `user_tour_id` INT(4) NOT NULL                      COMMENT '巡回ID',

    PRIMARY KEY(`user_name`, `user_tour_id`),
    FOREIGN KEY(`user_tour_id`) REFERENCES tour_table(`tour_id`)
        ON UPDATE CASCADE ON DELETE CASCADE,
    FOREIGN KEY(`user_name`) REFERENCES user_manage_table(`user_name`)
        ON DELETE CASCADE
)ENGINE=InnoDB; 

/*
 * Table No.11-01
 *
 * group table.
*/
CREATE TABLE group_table(
    `group_id` INT(4) NOT NULL  AUTO_INCREMENT           COMMENT '群组ID',
    `tw_id` INT(4) NOT NULL                              COMMENT '电视墙ID',
    `group_name` VARCHAR(64) NOT NULL UNIQUE             COMMENT '群组名称', 

    PRIMARY KEY(`group_id`),
    FOREIGN KEY(`tw_id`) REFERENCES tw_table(`tw_id`)
        ON DELETE CASCADE
)ENGINE=InnoDB;

/*
 * Table No.11-02
 *
 * group step table.
*/
CREATE TABLE group_step_table(
    
    `step_no` INT(4) NOT NULL AUTO_INCREMENT             COMMENT '群组步号', 
    `group_id` INT(4) NOT NULL                           COMMENT '群组ID',
    `interval` INT(4) NOT NULL                           COMMENT '群组步驻留时间',
   
    PRIMARY KEY(`step_no`),     
    FOREIGN KEY(`group_id`) REFERENCES group_table(`group_id`)
        ON DELETE CASCADE
)ENGINE=InnoDB;

/*
 * Table No.11-03
 *
 * group step info table.
*/
CREATE TABLE group_step_info_table(
    `step_no` INT(4) NOT NULL                            COMMENT '群组步号',
    `scr_id` INT(4) NOT NULL                             COMMENT '屏幕ID',
    `div_no` INT(4) NOT NULL                             COMMENT '分割号',
    `div_id` INT(4) NOT NULL                             COMMENT '分屏模式ID',
    `encoder_domain` CHAR(8) NOT NULL                    COMMENT '群组步对应的编码器所在的域',
    `encoder_guid` CHAR(24) NOT NULL                     COMMENT '群组步对应的编码器ID',
    `level` INT(4) NOT NULL DEFAULT 0                    COMMENT '码流类型',

    PRIMARY KEY(`step_no`, `scr_id`, `div_no`),
    FOREIGN KEY(`step_no`) REFERENCES group_step_table(`step_no`)
        ON DELETE CASCADE,
    FOREIGN KEY(`scr_id`) REFERENCES tw_screens_table(`scr_id`)
        ON DELETE CASCADE,
    FOREIGN KEY(`div_id`) REFERENCES tw_scr_division_table(`div_id`)
        ON DELETE CASCADE,
    FOREIGN KEY(`encoder_guid`, `encoder_domain`) REFERENCES dev_guconf_table(`gu_id`, `gu_domain`)
        ON UPDATE CASCADE ON DELETE CASCADE
)ENGINE=InnoDB;


/*
 * Table No.12-01
 *
 * alarm unit table.
*/
CREATE TABLE alarm_unit_table(
    `ams_id` CHAR(16) NOT NULL                          COMMENT '告警服务器ID',
    `ams_name` VARCHAR(64) NOT NULL UNIQUE              COMMENT '告警服务器名称',
    `ams_keep_alive_freq` INT(4) NOT NULL               COMMENT '心跳周期',
    `ams_state` INT(4) NOT NULL                         COMMENT '告警服务器在线状态',
    `ams_last_ip` VARCHAR(128)                          COMMENT '告警服务器上一次注册IP',

    PRIMARY KEY(`ams_id`)
)ENGINE=InnoDB; 

INSERT INTO alarm_unit_table VALUES('AMS-0001', '告警服务器',30,0,NULL);

/*
 * Table No.12-02
 *
 * link time policy table.
*/
CREATE TABLE link_time_policy_table(
    `gu_id` VARCHAR(24) NOT NULL                        COMMENT '告警业务点', 
    `domain_id` CHAR(8) NOT NULL                        COMMENT '告警业务点所属的域', 
    `time_policy` VARCHAR(16384) NOT NULL               COMMENT '告警时间策略',

    PRIMARY KEY(`gu_id`, `domain_id`),
    FOREIGN KEY(`gu_id`, `domain_id`) REFERENCES dev_guconf_table(`gu_id`, `gu_domain`)
        ON UPDATE CASCADE ON DELETE CASCADE
)ENGINE=InnoDB; 

/*
 * Table No.12-03
 *
 * link record table.
*/
CREATE TABLE link_record_table(
    `gu_id` VARCHAR(24) NOT NULL                        COMMENT '告警业务点',
    `domain_id` CHAR(8) NOT NULL                        COMMENT '告警业务点所属的域',
    `link_guid` VARCHAR(24) NOT NULL                    COMMENT '联动录像业务点',
    `link_domain_id` CHAR(8) NOT NULL                   COMMENT '联动录像业务点所属的域',
    `mss_id` CHAR(16) NOT NULL                          COMMENT '存储服务器ID',
    `time_len` INT(4) NOT NULL                          COMMENT '录像时长',
    `alarm_type` INT(4) NOT NULL                        COMMENT '告警类型',
    `level` INT(4) NOT NULL DEFAULT 0                   COMMENT '联动业务点的码流类型',

    PRIMARY KEY(`gu_id`, `domain_id`,`link_guid`, `link_domain_id`, `mss_id`),
    FOREIGN KEY(`gu_id`, `domain_id`) REFERENCES dev_guconf_table(`gu_id`, `gu_domain`)
        ON UPDATE CASCADE ON DELETE CASCADE,
    FOREIGN KEY(`link_guid`, `link_domain_id`,`mss_id`) REFERENCES record_policy_table(`gu_id`, `guid_domain`,`mss_id`)
        ON UPDATE CASCADE ON DELETE CASCADE
)ENGINE=InnoDB; 

/*
 * Table No.12-04
 *
 * link step table.
*/
CREATE TABLE link_step_table(
    `gu_id` VARCHAR(24) NOT NULL                        COMMENT '告警业务点',
    `domain_id` CHAR(8) NOT NULL                        COMMENT '告警业务点所属的域',
    `tw_id` INT(4) NOT NULL                             COMMENT '电视墙ID',
    `screen_id` INT(4) NOT NULL                         COMMENT '屏幕ID',
    `division_num` INT(4) NOT NULL                      COMMENT '分割号',
    `division_id` INT(4) NOT NULL                       COMMENT '分屏模式',
    `enc_guid` VARCHAR(24) NOT NULL                     COMMENT '联动业务点',
    `enc_domain_id` CHAR(8) NOT NULL                    COMMENT '联动业务点所属的域',
    `level` INT(4) NOT NULL DEFAULT 0                   COMMENT '联动业务点的码流类型',
    `alarm_type` INT(4) NOT NULL                        COMMENT '告警类型',

    PRIMARY KEY(`gu_id`, `domain_id`,`tw_id`, `screen_id`, `division_num`),
    FOREIGN KEY(`gu_id`, `domain_id`) REFERENCES dev_guconf_table(`gu_id`, `gu_domain`)
        ON UPDATE CASCADE ON DELETE CASCADE,
    FOREIGN KEY(`tw_id`, `screen_id`) REFERENCES tw_screens_table(`tw_id`, `scr_id`)
        ON UPDATE CASCADE ON DELETE CASCADE,
    FOREIGN KEY(`division_id`) REFERENCES tw_scr_division_table(`div_id`)
        ON UPDATE CASCADE ON DELETE CASCADE,
    FOREIGN KEY(`enc_guid`, `enc_domain_id`) REFERENCES dev_guconf_table(`gu_id`, `gu_domain`)
        ON UPDATE CASCADE ON DELETE CASCADE
)ENGINE=InnoDB; 


/*
 * Table No.12-05
 *
 * link io table.
*/
CREATE TABLE link_io_table(
    `gu_id` VARCHAR(24) NOT NULL                        COMMENT '告警业务点',
    `domain_id` CHAR(8) NOT NULL                        COMMENT '告警业务点所属的域',
    `link_guid` VARCHAR(24) NOT NULL                    COMMENT '联动录像业务点',
    `link_domain_id` CHAR(8) NOT NULL                   COMMENT '联动录像业务点所属的域',
    `IO_value` VARCHAR(64) DEFAULT NULL                 COMMENT '告警值',
    `time_len` INT(4) NOT NULL                          COMMENT '告警时长',
    `alarm_type` INT(4) NOT NULL                        COMMENT '告警类型',

    PRIMARY KEY(`gu_id`, `domain_id`,`link_guid`, `link_domain_id`),
    FOREIGN KEY(`gu_id`, `domain_id`) REFERENCES dev_guconf_table(`gu_id`, `gu_domain`)
        ON UPDATE CASCADE ON DELETE CASCADE,
    FOREIGN KEY(`link_guid`, `link_domain_id`) REFERENCES dev_guconf_table(`gu_id`, `gu_domain`)
        ON UPDATE CASCADE ON DELETE CASCADE
)ENGINE=InnoDB;

/*
 * Table No.12-06
 *
 * link snapshot table.
*/
CREATE TABLE link_snapshot_table(
    `gu_id` VARCHAR(24) NOT NULL                        COMMENT '告警业务点',
    `domain_id` CHAR(8) NOT NULL                        COMMENT '告警业务点所属的域',
    `link_guid` VARCHAR(24) NOT NULL                    COMMENT '联动抓拍业务点',
    `link_domain_id` CHAR(8) NOT NULL                   COMMENT '联动抓拍业务点所属的域',
    `level` INT(4) NOT NULL DEFAULT 0                   COMMENT '联动业务点的码流类型',
    `mss_id` CHAR(16) NOT NULL                          COMMENT '存储服务器ID',
    `picture_num` INT(4) NOT NULL                       COMMENT '抓拍图片数',
    `alarm_type` INT(4) NOT NULL                        COMMENT '告警类型',

    PRIMARY KEY(`gu_id`, `domain_id`,`link_guid`, `link_domain_id`, `mss_id`),
    FOREIGN KEY(`gu_id`, `domain_id`) REFERENCES dev_guconf_table(`gu_id`, `gu_domain`)
        ON UPDATE CASCADE ON DELETE CASCADE,
    FOREIGN KEY(`link_guid`, `link_domain_id`,`mss_id`) REFERENCES record_policy_table(`gu_id`, `guid_domain`,`mss_id`)
        ON UPDATE CASCADE ON DELETE CASCADE
)ENGINE=InnoDB; 

/*
 * Table No.12-07
 *
 * link preset table.
*/
CREATE TABLE link_preset_table(
    `gu_id` VARCHAR(24) NOT NULL                        COMMENT '告警业务点',
    `domain_id` CHAR(8) NOT NULL                        COMMENT '告警业务点所属的域',
    `link_guid` VARCHAR(24) NOT NULL                    COMMENT '联动业务点',
    `link_domain_id` CHAR(8) NOT NULL                   COMMENT '联动业务点所属的域',
    `preset_no` INT(4) NOT NULL                         COMMENT '预置位',
    `alarm_type` INT(4) NOT NULL                        COMMENT '告警类型',

    PRIMARY KEY(`gu_id`, `domain_id`,`link_guid`, `link_domain_id`),
    FOREIGN KEY(`gu_id`, `domain_id`) REFERENCES dev_guconf_table(`gu_id`, `gu_domain`)
        ON UPDATE CASCADE ON DELETE CASCADE,
    FOREIGN KEY(`link_guid`, `link_domain_id`) REFERENCES dev_guconf_table(`gu_id`, `gu_domain`)
        ON UPDATE CASCADE ON DELETE CASCADE
)ENGINE=InnoDB;

/*
 * Table No.12-08
 *
 * link map table.
*/
CREATE TABLE link_map_table(
    `gu_id` VARCHAR(24) NOT NULL                        COMMENT '告警业务点',
    `domain_id` CHAR(8) NOT NULL                        COMMENT '告警业务点所属的域',
    `link_guid` VARCHAR(24) NOT NULL                    COMMENT '联动录像业务点',
    `link_domain_id` CHAR(8) NOT NULL                   COMMENT '联动录像业务点所属的域',
    `level` INT(4) NOT NULL DEFAULT 0                   COMMENT '联动业务点的码流类型',
    `alarm_type` INT(4) NOT NULL                        COMMENT '告警类型',

    PRIMARY KEY(`gu_id`, `domain_id`,`link_guid`, `link_domain_id`),
    FOREIGN KEY(`gu_id`, `domain_id`) REFERENCES dev_guconf_table(`gu_id`, `gu_domain`)
        ON UPDATE CASCADE ON DELETE CASCADE,
    FOREIGN KEY(`link_guid`, `link_domain_id`) REFERENCES dev_guconf_table(`gu_id`, `gu_domain`)
        ON UPDATE CASCADE ON DELETE CASCADE
)ENGINE=InnoDB;


/*
 * Table No.13-01
 *
 * log table.
*/
CREATE TABLE log_table(
    `order_num` INT(4) NOT NULL AUTO_INCREMENT          COMMENT '序号',
    `log_time` INT(4) UNSIGNED NOT NULL                 COMMENT '写日志时间',
    `log_time_str` DATETIME NOT NULL                    COMMENT '写日志时间字符串',
    `log_level` INT(4) NOT NULL                         COMMENT '日志等级', 
    `user_name` VARCHAR(64) NOT NULL                    COMMENT '用户名称', 
    `operate_id` INT(4) NOT NULL                        COMMENT '操作ID',
    `result_code` INT(4) NOT NULL                       COMMENT '结果码',
    `child_type1` VARCHAR(64) DEFAULT NULL              COMMENT '操作子类型1',
    `child_type2` VARCHAR(64) DEFAULT NULL              COMMENT '操作子类型2',
    `child_type3` VARCHAR(64) DEFAULT NULL              COMMENT '操作子类型3',

    PRIMARY KEY(`order_num`)
)ENGINE=InnoDB;


/*
 * Table No.13-02
 *
 * area device online status table.
*/
CREATE TABLE area_dev_online_status_table(
    `order_id` INT(4) NOT NULL AUTO_INCREMENT              COMMENT '序号',
    `statistics_time` date NOT NULL  default '1970-01-01'  COMMENT '统计时间',
    `area_id` INT(4) NOT NULL                              COMMENT '区域', 
    `pu_total_count` DOUBLE(8,4) NOT NULL                  COMMENT '该区域的设备总数',
    `online_count` DOUBLE(8,4) NOT NULL                    COMMENT '该区域在线设备数',
    `statistics_type` INT(4) NOT NULL                      COMMENT '统计类型年0:某个时刻，1：某天，2：月，3：年',
  
    PRIMARY KEY(`order_id`),
    FOREIGN KEY(`area_id`) REFERENCES domain_area_table(`area_id`)
        ON DELETE CASCADE
)ENGINE=InnoDB;


/*
 * Table No.14-01
 *
 * intelligence analyse table.
*/
CREATE TABLE intelligence_analyse_table(
    `ivs_id` CHAR(16) NOT NULL                          COMMENT '智能分析服务器ID',
    `ivs_name` VARCHAR(64) NOT NULL UNIQUE              COMMENT '智能分析服务器名称',
    `ivs_keep_alive_freq` INT(4) NOT NULL               COMMENT '心跳周期',
    `ivs_state` INT(4) NOT NULL                         COMMENT '智能分析服务器在线状态',
    `ivs_last_ip` VARCHAR(128)                          COMMENT '智能分析服务器上一次注册IP',
    
    PRIMARY KEY(`ivs_id`)
)ENGINE=InnoDB; 

INSERT INTO intelligence_analyse_table VALUES('IVS-0001', '智能分析',30,0,NULL);


/*
 * Table No.15-01
 *
 * alarm device configure table.
*/
CREATE TABLE alarm_dev_configure_table(
    `pu_id` CHAR(16) NOT NULL                           COMMENT '设备ID号',
    `pu_domain` CHAR(8) NOT NULL                        COMMENT '设备所属的域',
    `ams_id` CHAR(16) NOT NULL                          COMMENT '告警服务器ID',
    `dev_name` VARCHAR(64) NOT NULL                     COMMENT '告警主机用户名',
    `dev_passwd` VARCHAR(64) NOT NULL                   COMMENT '告警主机密码',
    `dev_ip` VARCHAR(128)                               COMMENT '告警主机IP',
    `dev_port` INT(4)                                   COMMENT '告警主机端口',

    PRIMARY KEY(`ams_id`, `pu_id`, `pu_domain`),
    FOREIGN KEY(`pu_id`, `pu_domain`) REFERENCES dev_configure_table(`pu_id`, `pu_domain`)
        ON UPDATE CASCADE ON DELETE CASCADE,
    FOREIGN KEY(`ams_id`) REFERENCES alarm_unit_table(`ams_id`)
        ON DELETE CASCADE
)ENGINE=InnoDB;


CREATE TABLE user_guconf_number_table(
    `user_name` VARCHAR(64) BINARY NOT NULL             COMMENT '用户名',
    `user_gu_num` INT(4) NOT NULL                       COMMENT '用户的业务点编号',
    `user_guid` CHAR(24) NOT NULL                       COMMENT '用户所属业务点',
    `user_guid_domain` CHAR(8) NOT NULL                 COMMENT '该业务点对应的域',
    `user_guid_level` INT(4) NOT NULL                   COMMENT '该业务点的码流类型',

    PRIMARY KEY(`user_name`, `user_guid`, `user_guid_domain`),
    CONSTRAINT uk_user_gu_num UNIQUE (`user_name`, `user_gu_num`),
    FOREIGN KEY(`user_guid`, `user_guid_domain`) REFERENCES dev_guconf_table(`gu_id`, `gu_domain`)
        ON UPDATE CASCADE ON DELETE CASCADE,
    FOREIGN KEY(`user_name`) REFERENCES user_manage_table(`user_name`)
        ON DELETE CASCADE
)ENGINE=InnoDB;


CREATE TABLE user_screen_number_table(
    `user_name` VARCHAR(64) BINARY NOT NULL             COMMENT '用户名',
    `user_screen_num` INT(4) NOT NULL                   COMMENT '用户的屏幕编号',
    `screen_id` INT(4) NOT NULL                         COMMENT '屏幕ID',

    PRIMARY KEY(`user_name`, `screen_id`),
    CONSTRAINT uk_user_screen_num UNIQUE (`user_name`, `user_screen_num`),
    FOREIGN KEY(`screen_id`) REFERENCES tw_screens_table(`scr_id`)
        ON DELETE CASCADE,
    FOREIGN KEY(`user_name`) REFERENCES user_manage_table(`user_name`)
        ON DELETE CASCADE
)ENGINE=InnoDB;


CREATE TABLE user_tour_number_table(
    `user_name` VARCHAR(64) BINARY NOT NULL             COMMENT '用户名',
    `user_tour_num` INT(4) NOT NULL                     COMMENT '用户的巡回编号',
    `tour_id` INT(4) NOT NULL                           COMMENT '巡回ID',

    PRIMARY KEY(`user_name`, `tour_id`),
    CONSTRAINT uk_user_tour_num UNIQUE (`user_name`, `user_tour_num`),
    FOREIGN KEY(`tour_id`) REFERENCES tour_table(`tour_id`)
        ON DELETE CASCADE,
    FOREIGN KEY(`user_name`) REFERENCES user_manage_table(`user_name`)
        ON DELETE CASCADE
)ENGINE=InnoDB;


CREATE TABLE user_group_number_table(
    `user_name` VARCHAR(64) BINARY NOT NULL             COMMENT '用户名',
    `user_group_num` INT(4) NOT NULL                    COMMENT '用户的群组编号',
    `group_id` INT(4) NOT NULL                          COMMENT '群组ID',

    PRIMARY KEY(`user_name`, `group_id`),
    CONSTRAINT uk_user_group_num UNIQUE (`user_name`, `user_group_num`),
    FOREIGN KEY(`group_id`) REFERENCES group_table(`group_id`)
        ON DELETE CASCADE,
    FOREIGN KEY(`user_name`) REFERENCES user_manage_table(`user_name`)
        ON DELETE CASCADE
)ENGINE=InnoDB;


/* 
 * MySQL Procedures/Functions, For JXJ Platform DB.
*/


SET GLOBAL log_bin_trust_function_creators = 1;

DELIMITER //

DROP FUNCTION IF EXISTS get_dev_gus;    /* warning, Why ?? */
CREATE FUNCTION get_dev_gus(
    domain_id CHAR(8),
    pu_id CHAR(16)
)
RETURNS INT(4)
BEGIN
    DECLARE n_count INT(4);
    SELECT COUNT(*) INTO n_count
        FROM dev_guconf_table t
        WHERE t.gu_puid = pu_id AND t.gu_domain = domain_id;
    RETURN n_count;
END;


DROP PROCEDURE IF EXISTS get_area_all_device_count;    /* warning, Why ?? */
CREATE PROCEDURE get_area_all_device_count(
    IN root_area INT(4),
    OUT areas_count INT(4)
)
BEGIN
    CREATE TEMPORARY TABLE IF NOT EXISTS tmp_area_table(
        `idx` INT(4) AUTO_INCREMENT,
        `area_id` INT(4),
        `depth` INT(4),
        PRIMARY KEY(`idx`)
    );
    DELETE FROM tmp_area_table;
    SET max_sp_recursion_depth=128;
    CALL create_child_area_table(root_area, 0);
    SELECT COUNT(t2.pu_domain) INTO areas_count FROM tmp_area_table t1, dev_configure_table t2 WHERE t2.pu_area = t1.area_id;
    DROP TABLE tmp_area_table;
END;


DROP PROCEDURE IF EXISTS show_area_all_devices;    /* warning, Why ?? */
CREATE PROCEDURE show_area_all_devices(
    IN root_area INT(4),
    IN start INT(4),
    IN length INT(4)
)
BEGIN
    CREATE TEMPORARY TABLE IF NOT EXISTS tmp_area_table(
        `idx` INT(4) AUTO_INCREMENT,
        `area_id` INT(4),
        `depth` INT(4),
        PRIMARY KEY(`idx`)
    );
    DELETE FROM tmp_area_table;
    SET max_sp_recursion_depth=128;
    CALL create_child_area_table(root_area, 0);
    SET @sqlstr = CONCAT('SELECT t5.mdu_name,t6.*,t6.pu_state,t6.pu_id,t6.pu_last_ip,t6.pu_last_alive from dispatch_unit_table t5 right join \
       (SELECT t2.*, t2.pu_mdu as mdu, t4.mf_name, t3.area_name, t7.pu_state,t7.pu_last_ip,t7.pu_last_alive FROM tmp_area_table t1, dev_configure_table t2,\
       domain_area_table t3, dev_manufacturer_table t4, dev_running_info_table as t7 WHERE t2.pu_area = t1.area_id \
       AND t3.area_id = t1.area_id AND t2.pu_manufacturer = t4.mf_id AND t2.pu_id=t7.pu_id and t2.pu_domain=t7.pu_domain) t6 on \
       t6.mdu=t5.mdu_id order by t6.pu_state desc,t6.pu_id LIMIT ?,?');
    PREPARE stmt FROM @sqlstr;
        SET @a = start;
        SET @b = length;
        EXECUTE stmt USING @a, @b;
    DEALLOCATE PREPARE stmt;
    DROP TABLE tmp_area_table;
END;


DROP PROCEDURE IF EXISTS create_child_area_table;    /* warning, Why ?? */
CREATE PROCEDURE create_child_area_table(
    IN root_area INT(4),
    IN depth INT(4)
)
BEGIN
    DECLARE done INT DEFAULT 0;
    DECLARE child INT;
    DECLARE cur CURSOR FOR SELECT area_id from domain_area_table WHERE area_parent = root_area;
    DECLARE CONTINUE HANDLER FOR NOT FOUND SET done = 1;

    INSERT INTO tmp_area_table(`area_id`, `depth`) values(root_area, depth);
    OPEN cur;
    FETCH cur INTO child;
    WHILE done=0 DO
    CALL create_child_area_table(child, depth+1);
    FETCH cur INTO child;
    END WHILE; 
    CLOSE cur;
END;

DROP PROCEDURE IF EXISTS write_area_devices_status;    /* warning, Why ?? */
CREATE PROCEDURE write_area_devices_status(
    IN root_area INT(4)
)
BEGIN
    SET max_sp_recursion_depth=128;
    CALL write_area_dev_status_table(root_area, 0);
END;

DROP PROCEDURE IF EXISTS write_area_dev_status_table;    /* warning, Why ?? */
CREATE PROCEDURE write_area_dev_status_table(
    IN root_area INT(4),
    IN depth INT(4)
)
BEGIN
    DECLARE done INT DEFAULT 0;
    DECLARE child INT;
    DECLARE record_count INT;
    DECLARE total_count DOUBLE(8,4);
    DECLARE online DOUBLE(8,4);
    DECLARE cur_time date;
    DECLARE cur CURSOR FOR SELECT area_id from domain_area_table WHERE area_parent = root_area;
    DECLARE CONTINUE HANDLER FOR NOT FOUND SET done = 1;
    select count(*) into record_count from area_dev_online_status_table ;
    IF record_count > 20000
       THEN
       delete from area_dev_online_status_table where statistics_type = 1 order by order_id limit 500;
    END IF;
    select count(t2.pu_state) into online from dev_configure_table as t1, dev_running_info_table as t2 where t1.pu_area=root_area  and t1.pu_id=t2.pu_id and t1.pu_domain=t2.pu_domain and pu_state=1;
    select count(*) into total_count from dev_configure_table where pu_area=root_area;
    select CURRENT_DATE  into cur_time;
    INSERT INTO area_dev_online_status_table(`area_id`,`statistics_time`,`statistics_type`,`pu_total_count`, `online_count`) values(root_area,CURRENT_DATE,0, total_count,online);
    OPEN cur;
    FETCH cur INTO child;
    WHILE done=0 DO
    CALL write_area_dev_status_table(child, depth+1);
    FETCH cur INTO child;
    END WHILE; 
    CLOSE cur;
END;

DROP PROCEDURE IF EXISTS get_area_dev_online_status;    /* warning, Why ?? */
CREATE PROCEDURE get_area_dev_online_status(
    IN root_area INT(4),
    IN start INT(4),
    IN length INT(4)
)
BEGIN
CREATE TEMPORARY TABLE IF NOT EXISTS tmp_area_dev_online_table(
        `idx` INT(4) AUTO_INCREMENT,
        `area_id` INT(4),
        `total_count` DOUBLE(8,4),
        `online_count` DOUBLE(8,4),
        `depth` INT(4),
        PRIMARY KEY(`idx`)
    );
    DELETE FROM tmp_area_dev_online_table;
    SET max_sp_recursion_depth=128;
    CALL create_area_dev_online_table(root_area, 0);
    SET @sqlstr = CONCAT('select t1.area_name,t1.area_id,t2.online_count,t2.total_count,t2.online_count/t2.total_count as rate from domain_area_table t1,tmp_area_dev_online_table t2 where t1.area_id=t2.area_id limit ?,?');

    PREPARE stmt FROM @sqlstr;
        SET @a = start;
        SET @b = length;
        EXECUTE stmt USING @a, @b;
    DEALLOCATE PREPARE stmt;
    DROP TABLE tmp_area_dev_online_table;
END;


DROP PROCEDURE IF EXISTS check_area_dev_online_rate;    /* warning, Why ?? */
CREATE PROCEDURE check_area_dev_online_rate(
    IN root_area INT(4),
    IN depth INT(4)
)
BEGIN
    DECLARE done INT DEFAULT 0;
    DECLARE child INT;
    DECLARE total INT;
    DECLARE total_count DOUBLE(8,4);
    DECLARE online DOUBLE(8,4);
    DECLARE cur_time date;
    DECLARE cur CURSOR FOR SELECT area_id from domain_area_table WHERE area_parent = root_area;
    DECLARE CONTINUE HANDLER FOR NOT FOUND SET done = 1;
  
    select count(*), AVG(online_count),AVG(pu_total_count),statistics_time into total, online,total_count,cur_time from area_dev_online_status_table where area_id=root_area and statistics_type=0 and to_days(now())!=to_days(statistics_time);
    IF total != 0
        THEN
        select count(*) into total from area_dev_online_status_table WHERE area_id=root_area and statistics_type=1 and to_days(cur_time)=to_days(statistics_time); 
        IF total = 0 THEN
           INSERT INTO area_dev_online_status_table(`area_id`,`statistics_time`,`statistics_type`,`pu_total_count`, `online_count`) values(root_area,cur_time,1, total_count,online);
           DELETE FROM area_dev_online_status_table WHERE area_id=root_area and statistics_type=0 and to_days(now())!=to_days(statistics_time);
        ELSE
           UPDATE area_dev_online_status_table set pu_total_count=total_count,online_count=online WHERE area_id=root_area and statistics_type=1 and to_days(cur_time)=to_days(statistics_time);
           DELETE FROM area_dev_online_status_table WHERE area_id=root_area and statistics_type=0 and to_days(now())!=to_days(statistics_time);
        END IF;
    END IF;    
     
    OPEN cur;
    FETCH cur INTO child;
    WHILE done=0 DO
    CALL check_area_dev_online_rate(child, depth+1);
    FETCH cur INTO child;
    END WHILE; 
    CLOSE cur;
END;

DROP PROCEDURE IF EXISTS check_area_online_rate;    /* warning, Why ?? */
CREATE PROCEDURE check_area_online_rate(
    IN root_area INT(4)
)
BEGIN
    SET max_sp_recursion_depth=128;
    CALL check_area_dev_online_rate(root_area, 0);
END;

DROP PROCEDURE IF EXISTS create_area_dev_online_table;    /* warning, Why ?? */
CREATE PROCEDURE create_area_dev_online_table(
    IN root_area INT(4),
    IN depth INT(4)
)
BEGIN
    DECLARE done INT DEFAULT 0;
    DECLARE child INT;
    DECLARE online_num INT;
    DECLARE total_num INT;
    DECLARE cur CURSOR FOR SELECT area_id from domain_area_table WHERE area_parent = root_area;
    DECLARE CONTINUE HANDLER FOR NOT FOUND SET done = 1;
    select count(*) into total_num from dev_configure_table where pu_area=root_area;
    select count(t2.pu_state) into online_num from dev_configure_table as t1,dev_running_info_table as t2 where t1.pu_area=root_area and t1.pu_id=t2.pu_id and t1.pu_domain=t2.pu_domain and t2.pu_state=1;
    INSERT INTO tmp_area_dev_online_table(`area_id`,`total_count`,`online_count`, `depth`) values(root_area,total_num,online_num, depth);
    OPEN cur;
    FETCH cur INTO child;
    WHILE done=0 DO
    CALL create_area_dev_online_table(child, depth+1);
    FETCH cur INTO child;
    END WHILE; 
    CLOSE cur;
END;


 
DROP PROCEDURE IF EXISTS write_area_devices_status_day;    /* warning, Why ?? */
CREATE PROCEDURE write_area_devices_status_day(
    IN root_area INT(4)
)
BEGIN
    SET max_sp_recursion_depth=128;
    CALL write_area_dev_status_day_table(root_area, 0);
END;

DROP PROCEDURE IF EXISTS write_area_dev_status_day_table;    /* warning, Why ?? */
CREATE PROCEDURE write_area_dev_status_day_table(
    IN root_area INT(4),
    IN depth INT(4)
)
BEGIN
    DECLARE done INT DEFAULT 0;
    DECLARE child INT;
    DECLARE total INT DEFAULT 0;
    DECLARE total_count DOUBLE(8,4) DEFAULT 0;
    DECLARE online DOUBLE(8,4) DEFAULT 0;
    DECLARE cur_time date;
    DECLARE cur CURSOR FOR SELECT area_id from domain_area_table WHERE area_parent = root_area;
    DECLARE CONTINUE HANDLER FOR NOT FOUND SET done = 1;
    select count(*),AVG(online_count),AVG(pu_total_count) into total,online,total_count from area_dev_online_status_table where area_id=root_area and statistics_type=0 and to_days(now())-to_days(statistics_time)<=1;
    IF total != 0
      THEN
      select date_sub(curdate(), interval 1 day)  into cur_time;
      select count(*) into total from area_dev_online_status_table  WHERE area_id=root_area and statistics_type=1 and statistics_time = cur_time;
      IF total = 0
        THEN
        INSERT INTO area_dev_online_status_table(`area_id`,`statistics_time`,`pu_total_count`,`online_count`,`statistics_type`) values(root_area,cur_time,total_count,online,1);
      ELSE
        UPDATE area_dev_online_status_table set pu_total_count=total_count,online_count=online where area_id=root_area and statistics_type=1 and statistics_time = cur_time;
      END IF;
      DELETE FROM area_dev_online_status_table WHERE area_id=root_area and statistics_type=0 and to_days(now())-to_days(statistics_time)<=1;
    END IF;
    OPEN cur;
    FETCH cur INTO child;
    WHILE done=0 DO
    CALL write_area_dev_status_day_table(child, depth+1);
    FETCH cur INTO child;
    END WHILE; 
    CLOSE cur;
END;

DROP PROCEDURE IF EXISTS write_area_devices_status_month;    /* warning, Why ?? */
CREATE PROCEDURE write_area_devices_status_month(
    IN root_area INT(4)
)
BEGIN
    SET max_sp_recursion_depth=128;
    CALL write_area_dev_status_month_table(root_area, 0);
END;

DROP PROCEDURE IF EXISTS write_area_dev_status_month_table;    /* warning, Why ?? */
CREATE PROCEDURE write_area_dev_status_month_table(
    IN root_area INT(4),
    IN depth INT(4)
)
BEGIN
    DECLARE done INT DEFAULT 0;
    DECLARE child INT;
    DECLARE total INT;
    DECLARE total_count DOUBLE(8,4);
    DECLARE online DOUBLE(8,4);
    DECLARE cur_time date;
    DECLARE cur CURSOR FOR SELECT area_id from domain_area_table WHERE area_parent = root_area;
    DECLARE CONTINUE HANDLER FOR NOT FOUND SET done = 1;
    select count(*),AVG(online_count)  into total,online from area_dev_online_status_table where area_id=root_area and statistics_type=1 and period_diff(date_format(now(),'%Y%m'),date_format(statistics_time,'%Y%m'))=1;
    select AVG(pu_total_count) into total_count from area_dev_online_status_table where area_id=root_area and statistics_type=1 and  period_diff(date_format(now(),'%Y%m'),date_format(statistics_time,'%Y%m'))=1;
    IF total != 0
       THEN
       select date_sub(curdate(), interval 1 day)  into cur_time;
       select count(*) into total from area_dev_online_status_table  WHERE area_id=root_area and statistics_type=2 and period_diff(date_format(now(),'%Y%m'),date_format(statistics_time,'%Y%m'))=1;
       IF total = 0
         THEN
         INSERT INTO area_dev_online_status_table(`area_id`, `pu_total_count`, `online_count`,`statistics_time`, `statistics_type`) values(root_area, total_count,online,cur_time, 2);   
       ELSE
         UPDATE area_dev_online_status_table set pu_total_count=total_count,online_count=online where area_id=root_area and statistics_type=2 and period_diff(date_format(now(),'%Y%m'),date_format(statistics_time,'%Y%m'))=1;
       END IF;
    END IF;
    OPEN cur;
    FETCH cur INTO child;
    WHILE done=0 DO
    CALL write_area_dev_status_month_table(child, depth+1);
    FETCH cur INTO child;
    END WHILE; 
    CLOSE cur;
END;

DROP PROCEDURE IF EXISTS write_area_devices_status_year;    /* warning, Why ?? */
CREATE PROCEDURE write_area_devices_status_year(
    IN root_area INT(4)
)
BEGIN
    SET max_sp_recursion_depth=128;
    CALL write_area_dev_status_year_table(root_area, 0);
END;

DROP PROCEDURE IF EXISTS write_area_dev_status_year_table;    /* warning, Why ?? */
CREATE PROCEDURE write_area_dev_status_year_table(
    IN root_area INT(4),
    IN depth INT(4)
)
BEGIN
    DECLARE done INT DEFAULT 0;
    DECLARE child INT;
    DECLARE total INT;
    DECLARE total_count DOUBLE(8,4);
    DECLARE online DOUBLE(8,4);
    DECLARE cur_time date;
    DECLARE cur CURSOR FOR SELECT area_id from domain_area_table WHERE area_parent = root_area;
    DECLARE CONTINUE HANDLER FOR NOT FOUND SET done = 1;
    select count(*),AVG(pu_total_count) into total,total_count from area_dev_online_status_table where area_id=root_area and statistics_type=2 and YEAR(statistics_time)=YEAR(curdate());
    select AVG(online_count) into online from area_dev_online_status_table where area_id=root_area and statistics_type=2 and YEAR(statistics_time)=YEAR(curdate());
    IF total != 0
       THEN
       select CURRENT_DATE  into cur_time;
       select count(*) into total from area_dev_online_status_table  WHERE area_id=root_area and statistics_type=3 and YEAR(statistics_time)=YEAR(curdate());
       IF total = 0
         THEN
         INSERT INTO area_dev_online_status_table(`area_id`, `pu_total_count`, `online_count`,`statistics_time`, `statistics_type`) values(root_area, total_count, online, cur_time,3);   
       ELSE
         UPDATE area_dev_online_status_table set pu_total_count=total_count,online_count=online where area_id=root_area and statistics_type=3 and YEAR(statistics_time)=YEAR(curdate());
       END IF;
    END IF;
    OPEN cur;
    FETCH cur INTO child;
    WHILE done=0 DO
    CALL write_area_dev_status_year_table(child, depth+1);
    FETCH cur INTO child;
    END WHILE; 
    CLOSE cur;
END;

DROP PROCEDURE IF EXISTS clear_db_tables;    /* warning, Why ?? */
CREATE PROCEDURE clear_db_tables()
BEGIN
    DECLARE done INT DEFAULT 0;
    DECLARE tb_name VARCHAR(64);
    DECLARE cur CURSOR FOR SELECT table_name FROM information_schema.TABLES WHERE table_schema = 'jxj_platform_db';
    DECLARE CONTINUE HANDLER FOR SQLSTATE '02000' SET done = 1;

    OPEN cur;
    FETCH cur INTO tb_name;
    WHILE done=0 DO
        SET @sqlstr = CONCAT('TRUNCATE jxj_platform_db.', tb_name);
        PREPARE stmt FROM @sqlstr;
            EXECUTE stmt;
        DEALLOCATE PREPARE stmt;
        FETCH cur INTO tb_name;
    END WHILE;
    CLOSE cur;
END;


DROP TRIGGER IF EXISTS update_mds_of_pu; /* warning, Why ?? */
CREATE TRIGGER update_mds_of_pu AFTER DELETE ON dispatch_unit_table
FOR EACH ROW
BEGIN
    UPDATE dev_configure_table SET pu_mdu=NULL where pu_mdu=OLD.mdu_id;
END;


DROP TRIGGER IF EXISTS update_alarm_id; /* warning, Why ?? */
CREATE TRIGGER update_alarm_id AFTER INSERT ON alarm_info_table
FOR EACH ROW
BEGIN
    IF @@global.connect_timeout != 127
    THEN
        INSERT INTO param_config_table values(2, NEW.alarm_id) ON DUPLICATE KEY UPDATE value=NEW.alarm_id;
    END IF;
END;

DROP TRIGGER IF EXISTS update_pu_id; /* warning, Why ?? */
CREATE TRIGGER update_pu_id AFTER INSERT ON dev_configure_table
FOR EACH ROW
BEGIN
    IF @@global.connect_timeout != 127
    THEN
        INSERT INTO param_config_table values(3, NEW.pu_id) ON DUPLICATE KEY UPDATE value=NEW.pu_id;
    END IF;
END; 

//

DELIMITER ;


set global event_scheduler =1;
DROP EVENT IF EXISTS EVENT_WRITE_AREA_DEV_STATUS_TIMER; /* warning, Why ?? */
CREATE EVENT EVENT_WRITE_AREA_DEV_STATUS_TIMER  
ON SCHEDULE EVERY 1 HOUR
STARTS TIMESTAMP '2013-07-03 00:00:00'
DO
CALL  write_area_devices_status(1);


DROP EVENT IF EXISTS EVENT_WRITE_AREA_DEV_STATUS_DAY; /* warning, Why ?? */
CREATE EVENT EVENT_WRITE_AREA_DEV_STATUS_DAY  
ON SCHEDULE EVERY 1 DAY
STARTS TIMESTAMP '2013-07-01 00:00:01'
DO
CALL  write_area_devices_status_day(1);

DROP EVENT IF EXISTS EVENT_WRITE_AREA_DEV_STATUS_MONTH; /* warning, Why ?? */
CREATE EVENT EVENT_WRITE_AREA_DEV_STATUS_MONTH  
ON SCHEDULE EVERY 1 MONTH
STARTS TIMESTAMP '2013-01-01 00:00:01'
DO
CALL  write_area_devices_status_month(1);

DROP EVENT IF EXISTS EVENT_WRITE_AREA_DEV_STATUS_YEAE; /* warning, Why ?? */
CREATE EVENT EVENT_WRITE_AREA_DEV_STATUS_YEAE  
ON SCHEDULE EVERY 1 YEAR
STARTS TIMESTAMP '2012-12-31 23:59:00'
DO
CALL  write_area_devices_status_year(1);
