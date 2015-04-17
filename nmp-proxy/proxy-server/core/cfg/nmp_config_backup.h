
#ifndef __CONFIG_BACKUP_H__
#define __CONFIG_BACKUP_H__

#include "nmplib.h"


#define DEF_BACKUP_MAGIC            0x6a786a2d
#define DEF_REVERT_MAGIC            0x6a786a2b

enum
{
    NORMAL,
    BACKUP,
    REVERT,
};

typedef struct config_backup config_backup_t;

struct config_backup
{
    nmp_mutex_t *lock;
    unsigned int state;
    unsigned int left;
};


#ifdef __cplusplus
extern "C" {
#endif

int init_config_backup();
config_backup_t *
    get_config_backup();

int get_backup_state(config_backup_t *cfg_bkp);
int set_backup_state(config_backup_t *cfg_bkp, int state);
int try_change_backup_state(config_backup_t *cfg_bkp, int state);

int get_backup_left_size(config_backup_t *cfg_bkp);
int set_backup_left_size(config_backup_t *cfg_bkp, int size);

int config_figure_up_file_len(config_backup_t *cfg_bkp);

int config_create_backup_listener(int *listener);
int config_start_backup_listen_thread(int listener);


#ifdef __cplusplus
    }
#endif


#endif  //__CONFIG_BACKUP_H__

