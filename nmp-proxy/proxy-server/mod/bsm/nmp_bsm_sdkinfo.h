#ifndef __BSM_SDKINFO_H__
#define __BSM_SDKINFO_H__


#define BSM_INVALID_HANDLE          0


typedef enum
{
    BSM_UNKNOWN=-1,
    BSM_LOGOUT = 0,
    BSM_LOGING = 1,
    BSM_LOGIN  = 2,
}BSM_STATE_E;

typedef enum
{
    BSM_PTZ_CTRL_STD,
    BSM_PTZ_CTRL_PRS,
    BSM_PTZ_CTRL_EXT,
}BSM_PTZ_CTRL_E;

typedef struct bsm_config bsm_config_t;

struct bsm_config
{
    int    command;
    int    channel;
    void  *buffer;
    size_t b_size;
};

typedef struct bsm_ptz_ctrl_std bsm_ptz_ctrl_std_t;

struct bsm_ptz_ctrl_std
{
    int cmd;
    int speed;
};

typedef struct bsm_ptz_ctrl_prs bsm_ptz_ctrl_prs_t;

struct bsm_ptz_ctrl_prs
{
    int cmd;
    int preset;
};

typedef struct bsm_ptz_ctrl_ext bsm_ptz_ctrl_ext_t;

struct bsm_ptz_ctrl_ext
{
    int cmd;
};


#endif  //__BSM_SDKINFO_H__


