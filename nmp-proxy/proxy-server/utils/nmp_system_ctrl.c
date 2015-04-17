
#include <glib.h>
#include <glib-object.h>

#include <fcntl.h>
#include <sys/resource.h>

#include "nmp_share_debug.h"
#include "nmp_share_signal.h"
#include "nmp_share_rw_file.h"

#include "nmp_system_ctrl.h"


#define SC_MAX_DIR_SIZE        128
#define SC_MAX_DEVICE_IP        32

#define SC_CONFIG_ENV_NAME      "PROXY_CONFIG_PATH"
#define SC_CONFIG_FILE_NAME     "jxnmp_server.conf"

#define SECTION_GLOBAL          "section.global"
#define SECTION_PROXY           "section.proxy"

#define LOG_DIR                 "log_dir"
#define LOG_MODE                "log_mode"
#define LOG_LEVEL               "log_level"
#define DATA_DIR                "data_dir"




typedef enum
{
    SC_LOG_PATH,
    SC_LOG_MODE,
    SC_LOG_LEVEL,
    SC_DATA_PATH,
}JSCID_E;



typedef struct sys_ctrl sys_ctrl_t;

struct sys_ctrl
{
    char log_path[SC_MAX_DIR_SIZE];
    char data_path[SC_MAX_DIR_SIZE];
    int  log_mode;                      /* 0:write to log file, 1:write to stdout*/
    int  log_level;                     /* 1~5*/
};

static sys_ctrl_t nmp_proxy_ctrl = 
{
    .log_path  = "./../var/log",
    .data_path = "./../var/proxy",
    .log_mode  = 1,
    .log_level = 1,
};



static __inline__ void *
__system_ctrl_get_value(sys_ctrl_t *sc, JSCID_E id)
{
    void *value = NULL;

    switch (id)
    {
        case SC_LOG_PATH:
            value = (void*)sc->log_path;
            break;

        case SC_LOG_MODE:
            value = (void*)sc->log_mode;
            break;

        case SC_LOG_LEVEL:
            value = (void*)sc->log_level;
            break;

        case SC_DATA_PATH:
            value = (void*)sc->data_path;
            break;

        default:
            BUG();
            break;
    }

    return value;
}

static __inline__ void
__system_ctrl_set_value(rw_file *conf, sys_ctrl_t *sc, JSCID_E id, gchar *value)
{
    char tmp[4];

    switch (id)
    {
        case SC_LOG_PATH:
            if (value)
                strncpy(sc->log_path, value, SC_MAX_DIR_SIZE - 1);

            set_value_of(conf, SECTION_GLOBAL, LOG_DIR, (const gchar*)sc->log_path);
            break;

        case SC_LOG_LEVEL:
            if (value)
            {
                if (!atoi(value))
                    sc->log_level = 0;
                else
                    sc->log_level = 1;
            }
            sprintf(tmp, "%d", sc->log_level);
            set_value_of(conf, SECTION_PROXY, LOG_LEVEL, (const gchar*)tmp);
            break;

        case SC_LOG_MODE:
            if (value)
            {
                if (!atoi(value))
                    sc->log_mode = 0;
                else
                    sc->log_mode = 1;
            }
            sprintf(tmp, "%d", sc->log_mode);
            set_value_of(conf, SECTION_PROXY, LOG_MODE, (const gchar*)tmp);
            break;

        case SC_DATA_PATH:
            if (value)
                strncpy(sc->data_path, value, SC_MAX_DIR_SIZE - 1);
            set_value_of(conf, SECTION_PROXY, DATA_DIR, (const gchar*)sc->data_path);
            break;

        default:
            BUG();
            break;
    }
}

static __inline__ void
__system_ctrl_init(sys_ctrl_t *sc, const gchar *env_name)
{
    gint err;
    rw_file *fd;
    gchar *path, file_name[SC_MAX_DIR_SIZE];

    path = getenv(env_name);
    if (!path)
    {
        nmp_error("<main> PROXY_CONFIG_PATH NULL!");
        FATAL_ERROR_EXIT;
    }

    if (strlen(path) + strlen("/") + strlen(SC_CONFIG_FILE_NAME) >= SC_MAX_DIR_SIZE)
    {
        nmp_warning("Proxy open 'jxnmp_server.conf' failed, path too long.");
        return ;        /* use default */
    }

    strcpy(file_name, path);
    strcat(file_name, "/");
    strcat(file_name, SC_CONFIG_FILE_NAME);

    fd = open_rw_file(file_name, O_RDONLY, &err);
    if (!fd)
    {
        nmp_warning("Proxy open 'jxnmp_server.conf' failed, err: %d.", err);
        return ;        /* use default */
    }

    __system_ctrl_set_value(fd, sc, SC_LOG_PATH, 
        (gchar*)get_value_of(fd, SECTION_GLOBAL, 0, LOG_DIR));
    __system_ctrl_set_value(fd, sc, SC_DATA_PATH, 
        (gchar*)get_value_of(fd, SECTION_PROXY, 0, DATA_DIR));
    __system_ctrl_set_value(fd, sc, SC_LOG_LEVEL, 
        (gchar*)get_value_of(fd, SECTION_PROXY, 0, LOG_LEVEL));
    __system_ctrl_set_value(fd, sc, SC_LOG_MODE, 
        (gchar*)get_value_of(fd, SECTION_PROXY, 0, LOG_MODE));

    flush_rw_file(fd);
    close_rw_file(fd);
}

static __inline__ void 
nmp_system_ctrl_init()
{
    __system_ctrl_init(&nmp_proxy_ctrl, SC_CONFIG_ENV_NAME);
}

static __inline__ void *
nmp_system_ctrl_get_value(JSCID_E id)
{
    return __system_ctrl_get_value(&nmp_proxy_ctrl, id);
}

static __inline__ void
nmp_proxy_setup_signals()
{
    signal(SIGPIPE, SIG_IGN);
    nmp_sig_setup_signals();
}

static __inline__ void
nmp_proxy_log_facility_init()
{
    nmp_debug_log_facility_init(nmp_system_ctrl_get_value(SC_LOG_PATH), "Jpf-proxy.log");
}

#ifdef _OPEN_CORE_
static __inline__ void
nmp_proxy_open_core_facility()
{
    struct rlimit rli;

    rli.rlim_cur = RLIM_INFINITY;
    rli.rlim_max = RLIM_INFINITY;

    if (setrlimit(RLIMIT_CORE, &rli) < 0)
    {
        nmp_error("<main> set core resource limit error!");
        FATAL_ERROR_EXIT;
    }
}
#endif

void
proxy_running_env_init()
{
#ifndef G_THREADS_ENABLED
    nmp_error("<main> CMS compiled without 'G_THREADS_ENABLED' defined!");
    FATAL_ERROR_EXIT;
#endif

    g_slice_set_config(G_SLICE_CONFIG_ALWAYS_MALLOC, TRUE);

    g_type_init();
    g_thread_init(NULL);

    nmp_system_ctrl_init();
    nmp_proxy_setup_signals();
    nmp_proxy_log_facility_init();

#ifdef _OPEN_CORE_
    nmp_proxy_open_core_facility();
#endif
}

void *proxy_get_data_file_path()
{
    return nmp_system_ctrl_get_value(SC_DATA_PATH);
}

//:~ End
