//***************************************************************************
// examples/asmp_extensions/cmd_parser.hpp
//
//   Data  : 2019/01/31
//   Author: Neusoft
//***************************************************************************
#ifndef cmd_parser_HPP_
#define cmd_parser_HPP_

#include "libgetopt.hpp"

#define SCRIPT_COMMAND_BUFFER_MAX 128
#define COMMAND_ARGV_MAX 128
#define LONG_OPTIONS_MAX 15



class CmdParser {
public:
    typedef bool (*CommandProcessFunc)(int nargc, char *const *nargv, const char *options,
            const struct option *long_options);
    struct CmdOption
    {
        const char *cmd;
        const char *optstring;
        CommandProcessFunc cmdProcessFunc;
        struct option long_options[LONG_OPTIONS_MAX];
    };

    ~CmdParser();
    bool processUserInput();
    bool checkQuitCmd();
    static CmdParser *pInstantce;
    static CmdParser *GetSingletonInstance();
    static void FreeSingletonInstance();

    static bool app_cmd_mq_init(int nargc, char *const *nargv, const char *options,
            const struct option *long_options);
    static bool app_cmd_mq_destroy(int nargc, char *const *nargv, const char *options,
            const struct option *long_options);
    static bool app_cmd_mq_send(int nargc, char *const *nargv, const char *options,
            const struct option *long_options);
    static bool app_cmd_mq_receive(int nargc, char *const *nargv, const char *options,
            const struct option *long_options);
    static bool app_cmd_mq_notify(int nargc, char *const *nargv, const char *options,
            const struct option *long_options);
    static bool app_cmd_mpmutex_init(int nargc, char *const *nargv, const char *options,
            const struct option *long_options);
    static bool app_cmd_mpmutex_destroy(int nargc, char *const *nargv, const char *options,
            const struct option *long_options);
    static bool app_cmd_mpmutex_lock(int nargc, char *const *nargv, const char *options,
            const struct option *long_options);
    static bool app_cmd_mpmutex_trylock(int nargc, char *const *nargv, const char *options,
            const struct option *long_options);
    static bool app_cmd_mpmutex_unlock(int nargc, char *const *nargv, const char *options,
            const struct option *long_options);
    static bool app_cmd_mpshm_init(int nargc, char *const *nargv, const char *options,
            const struct option *long_options);
    static bool app_cmd_mpshm_destroy(int nargc, char *const *nargv, const char *options,
            const struct option *long_options);
    static bool app_cmd_mpshm_attach(int nargc, char *const *nargv, const char *options,
            const struct option *long_options);
    static bool app_cmd_mpshm_detach(int nargc, char *const *nargv, const char *options,
            const struct option *long_options);
    static bool app_cmd_mpshm_virt2phys(int nargc, char *const *nargv, const char *options,
            const struct option *long_options);
    static bool app_cmd_mpshm_phys2virt(int nargc, char *const *nargv, const char *options,
            const struct option *long_options);
    static bool app_cmd_mpshm_control(int nargc, char *const *nargv, const char *options,
            const struct option *long_options);
    static bool app_cmd_mpshm_remap(int nargc, char *const *nargv, const char *options,
            const struct option *long_options);
    static bool app_cmd_task_init(int nargc, char *const *nargv, const char *options,
            const struct option *long_options);
    static bool app_cmd_task_init_secure(int nargc, char *const *nargv, const char *options,
            const struct option *long_options);
    static bool app_cmd_task_destroy(int nargc, char *const *nargv, const char *options,
            const struct option *long_options);
    static bool app_cmd_task_assign(int nargc, char *const *nargv, const char *options,
            const struct option *long_options);
    static bool app_cmd_task_bind(int nargc, char *const *nargv, const char *options,
            const struct option *long_options);
    static bool app_cmd_task_exec(int nargc, char *const *nargv, const char *options,
            const struct option *long_options);
    static bool app_cmd_task_join(int nargc, char *const *nargv, const char *options,
            const struct option *long_options);
    static bool app_cmd_task_attr_init(int nargc, char *const *nargv, const char *options,
            const struct option *long_options);
    static bool app_cmd_task_setattr(int nargc, char *const *nargv, const char *options,
            const struct option *long_options);
    static bool app_cmd_task_getattr(int nargc, char *const *nargv, const char *options,
            const struct option *long_options);
    static bool app_cmd_write_shm(int nargc, char *const *nargv, const char *options,
            const struct option *long_options);
    static bool app_cmd_read_shm(int nargc, char *const *nargv, const char *options,
            const struct option *long_options);
    static bool app_cmd_quit(int nargc, char *const *nargv, const char *options,
            const struct option *long_options);
private:
    CmdParser();
    char string[SCRIPT_COMMAND_BUFFER_MAX];
};

#endif
