//***************************************************************************
// examples/smp_extensions/cmd_parser.cxx
//
//   Data  : 2019/01/31
//   Author: Neusoft
//***************************************************************************
#include <sdk/config.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <debug.h>
#include <nuttx/init.h>

#include "cmd_parser.hpp"
#include "system/readline.h"
#include "test_api.hpp"
#include "asmp_extensions_common.hpp"

//define input cmd parms
static const struct CmdParser::CmdOption command_options[] =
{
    {"MQ_INIT",         "z", &CmdParser::app_cmd_mq_init,      {
            {"index", required_argument, NULL, 'i'},
            {"cpuid", required_argument, NULL, 'c'},
            {"keyid", required_argument, NULL, 'k'},
            {0, 0, 0, 0},
    }},
    {"MQ_DESTROY",      "z", &CmdParser::app_cmd_mq_destroy,   {
            {"index", required_argument, NULL, 'i'},
            {0, 0, 0, 0},
    }},
    {"MQ_SEND",      "z", &CmdParser::app_cmd_mq_send,         {
            {"index", required_argument, NULL, 'i'},
            {"msgid", required_argument, NULL, 'm'},
            {"value", required_argument, NULL, 'v'},
            {"read", no_argument, NULL, 'r'},
            {"write", no_argument, NULL, 'w'},
            {"lock", no_argument, NULL, 'l'},
            {"unlock", no_argument, NULL, 'u'},
            {"quit", no_argument, NULL, 'q'},
            {"send", no_argument, NULL, '1'},
            {"rev", no_argument, NULL, '2'},
            {0, 0, 0, 0},
    }},
    {"MQ_RECV",      "z", &CmdParser::app_cmd_mq_receive,      {
            {"index", required_argument, NULL, 'i'},
            {0, 0, 0, 0},
    }},
    {"MQ_NOTIFY",    "z", &CmdParser::app_cmd_mq_notify,       {
            {"index", required_argument, NULL, 'i'},
            {0, 0, 0, 0},
    }},
    {"MT_INIT",    "z", &CmdParser::app_cmd_mpmutex_init,       {
            {"index", required_argument, NULL, 'i'},
            {"keyid", required_argument, NULL, 'k'},
            {0, 0, 0, 0},
    }},
    {"MT_DESTROY",    "z", &CmdParser::app_cmd_mpmutex_destroy,       {
            {"index", required_argument, NULL, 'i'},
            {0, 0, 0, 0},
    }},
    {"MT_LOCK",    "z", &CmdParser::app_cmd_mpmutex_lock,       {
            {"index", required_argument, NULL, 'i'},
            {0, 0, 0, 0},
    }},
    {"MT_UNLOCK",    "z", &CmdParser::app_cmd_mpmutex_unlock,       {
            {"index", required_argument, NULL, 'i'},
            {0, 0, 0, 0},
    }},
    {"MT_TRYLOCK",    "z", &CmdParser::app_cmd_mpmutex_trylock,       {
            {"index", required_argument, NULL, 'i'},
            {0, 0, 0, 0},
    }},
    {"SHM_INIT",    "z", &CmdParser::app_cmd_mpshm_init,       {
            {"index", required_argument, NULL, 'i'},
            {"keyid", required_argument, NULL, 'k'},
            {"size", required_argument, NULL, 's'},
            {0, 0, 0, 0},
    }},
    {"SHM_DESTROY",    "z", &CmdParser::app_cmd_mpshm_destroy,       {
            {"index", required_argument, NULL, 'i'},
            {0, 0, 0, 0},
    }},
    {"SHM_ATTATCH",    "z", &CmdParser::app_cmd_mpshm_attach,       {
            {"index", required_argument, NULL, 'i'},
            {"flag", required_argument, NULL, 'f'},
            {0, 0, 0, 0},
    }},
    {"SHM_DETATCH",    "z", &CmdParser::app_cmd_mpshm_detach,       {
            {"index", required_argument, NULL, 'i'},
            {0, 0, 0, 0},
    }},
    {"SHM_V2P",    "z", &CmdParser::app_cmd_mpshm_virt2phys,       {
            {"index", required_argument, NULL, 'i'},
            {0, 0, 0, 0},
    }},
    {"SHM_P2V",    "z", &CmdParser::app_cmd_mpshm_phys2virt,       {
            {"index", required_argument, NULL, 'i'},
            {0, 0, 0, 0},
    }},
    {"SHM_CTRL",    "z", &CmdParser::app_cmd_mpshm_control,       {
            {"index", required_argument, NULL, 'i'},
            {"poweron", no_argument, NULL, 'n'},
            {"poweroff", no_argument, NULL, 'f'},
            {"retention", no_argument, NULL, 'r'},
            {0, 0, 0, 0},
    }},
    {"SHM_REMAP",    "z", &CmdParser::app_cmd_mpshm_remap,       {
            {"index", required_argument, NULL, 'i'},
            {0, 0, 0, 0},
    }},
    {"TASK_INIT",       "z", &CmdParser::app_cmd_task_init,    {
            {"index", required_argument, NULL, 'i'},
            {"filename", required_argument, NULL, 'f'},
            {0, 0, 0, 0},
    }},
    {"TASK_INIT_SEC",   "z", &CmdParser::app_cmd_task_init_secure, {
            {"index", required_argument, NULL, 'i'},
            {"filename", required_argument, NULL, 'f'},
            {0, 0, 0, 0},
    }},
    {"TASK_DESTROY",    "z", &CmdParser::app_cmd_task_destroy, {
            {"index", required_argument, NULL, 'i'},
            {0, 0, 0, 0},
    }},
    {"TASK_ASSIGN",     "z", &CmdParser::app_cmd_task_assign,  {
            {"index", required_argument, NULL, 'i'},
            {0, 0, 0, 0},
    }},
    {"TASK_BIND",       "z", &CmdParser::app_cmd_task_bind,    {
            {"index", required_argument, NULL, 'i'},
            {"mq", required_argument, NULL, 'm'},
            {"mutex", required_argument, NULL, 'x'},
            {"shm", required_argument, NULL, 's'},
            {0, 0, 0, 0},
    }},
    {"TASK_EXEC",       "z", &CmdParser::app_cmd_task_exec,    {
            {"index", required_argument, NULL, 'i'},
            {0, 0, 0, 0},
    }},
    {"TASK_JOIN",       "z", &CmdParser::app_cmd_task_exec,    {
            {"index", required_argument, NULL, 'i'},
            {0, 0, 0, 0},
    }},
    {"TASK_ATTR_INIT",  "z", &CmdParser::app_cmd_task_attr_init,    {
            {"index", required_argument, NULL, 'i'},
            {0, 0, 0, 0},
    }},
    {"TASK_SETATTR",  "z", &CmdParser::app_cmd_task_setattr,    {
            {"index", required_argument, NULL, 'i'},
            {0, 0, 0, 0},
    }},
    {"TASK_GETATTR",  "z", &CmdParser::app_cmd_task_getattr,    {
            {"index", required_argument, NULL, 'i'},
            {0, 0, 0, 0},
    }},

    {"WR_SHM",          "z", &CmdParser::app_cmd_write_shm,    {
            {"index", required_argument, NULL, 'i'},
            {"value", required_argument, NULL, 'v'},
            {0, 0, 0, 0},
    }},
    {"RD_SHM",          "z", &CmdParser::app_cmd_read_shm,     {
            {"index", required_argument, NULL, 'i'},
            {0, 0, 0, 0},
    }},
    {"QUIT"  ,          "z", &CmdParser::app_cmd_quit,         {
            {0, 0, 0, 0},
    }},
};


CmdParser *CmdParser::pInstantce = 0;
CmdParser *CmdParser::GetSingletonInstance()
{
    if(pInstantce == NULL)
    {
        pInstantce = new CmdParser();
    }
    return pInstantce;
}
void CmdParser::FreeSingletonInstance()
{
    if(pInstantce != NULL)
    {
        delete pInstantce;
        pInstantce = NULL;
    }
}

CmdParser::CmdParser()
{

}

CmdParser::~CmdParser()
{
}

bool CmdParser::processUserInput()
{
    bool ret = false;
    // get tester input from Serial
    memset(string,0,sizeof(string));
    int len = 0;
    do
    {
        len = readline(string, sizeof(string) - 1, stdin, stdout);
        string[len] = 0;
    }
    while(string[len - 1] != '\r' && string[len - 1] != '\n');
    //printf("cmd:len:%d, %x\n", len, string[len-1]);
    string[len - 1] = 0;

    //parse string to argc and argv
    int argc;
    char *argv[COMMAND_ARGV_MAX];
    char *pStr = string;
    //int strLength = sizeof(string);
    argc = 0;
    memset(argv,0,sizeof(argv));

    while(*pStr)
    {
        if(*pStr == ' ')
        {
            *pStr = 0;
            if(argv[argc] == 0 && *(pStr + 1) != ' ')
            {
                argv[argc++] = pStr + 1;
            }
        }
        else if(argc == 0)
        {
            argv[argc++] = pStr;
        }
        pStr++;
    }

    if (argv[0] == NULL)
        return true;

    //check input command index in command_options
    int commandIndex = 0;
    int commandTableMax = sizeof(command_options) / sizeof(CmdOption);
    while(commandIndex < commandTableMax)
    {
        if(strcmp(argv[0], command_options[commandIndex].cmd) == 0)
        {
            optind = 1;
            bool cmdResult = (*command_options[commandIndex].cmdProcessFunc)(argc,
                    argv,
                    command_options[commandIndex].optstring,
                    command_options[commandIndex].long_options);
            printf("%s\n",cmdResult?"TRUE":"FALSE");
            ret = true;
            break;
        }
        commandIndex++;
    }
    return true;
}

bool CmdParser::checkQuitCmd()
{
    if(string[0] == 'Q' && string[1] == 'U' && string[2] == 'I' &&
            string[3] == 'T' && string[4] == 0 )
    {
        return true;
    }
    return false;
}

bool CmdParser::app_cmd_mq_init(int nargc, char *const *nargv, const char *options,
        const struct option *long_options)
{
    int opt;
    optind = 1;
    int option_index = 0;
    int index = 0;
    int cpuid = 0;
    while ((opt = parse_arg_opt_long(nargc, nargv, options, long_options, &option_index)) != -1)
    {
        switch(opt)
        {
        case 'k':
            g_mptasks[index].mq_key = atoi(optarg);
            break;
        case 'i':
            index = atoi(optarg);
            break;
        case 'c':
            cpuid = atoi(optarg);
            break;
        }
    }
    bool ret = ASMP_mpmq_init(&(g_mptasks[index].mq), g_mptasks[index].mq_key, cpuid?cpuid:g_mptasks[index].cpuid);
    if (ret)
        printf("MsgQueue init succeed\n");
    else
        printf("MsgQueue init error\n");
    return ret;
}

bool CmdParser::app_cmd_mq_destroy(int nargc, char *const *nargv, const char *options,
        const struct option *long_options)
{
    int opt;
    optind = 1;
    int option_index = 0;
    int index = 0;
    while ((opt = parse_arg_opt_long(nargc, nargv, options, long_options, &option_index)) != -1)
    {
        switch(opt)
        {
        case 'i':
            index = atoi(optarg);
            break;
        }
    }
    bool ret = ASMP_mpmq_destroy(&(g_mptasks[index].mq));
    if (ret)
        printf("MsgQueue destroy succeed\n");
    else
        printf("MsgQueue destroy error\n");
    return ret;
}

bool CmdParser::app_cmd_mq_send(int nargc, char *const *nargv, const char *options,
        const struct option *long_options)
{
    int opt;
    optind = 1;
    int option_index = 0;
    int index = 0;
    int8_t msgid = 0;
    uint32_t data = 0;
    while ((opt = parse_arg_opt_long(nargc, nargv, options, long_options, &option_index)) != -1)
    {
        switch(opt)
        {
        case 'i':
            index = atoi(optarg);
            break;
        case 'v':
            data = atoi(optarg);
            break;
        case 'm':
            msgid = atoi(optarg);
            break;
        case 'r':
            msgid = MSG_ID_READ_SHM_START;
            break;
        case 'w':
            msgid = MSG_ID_WRITE_SHM_START;
            break;
        case 'q':
            msgid = MSG_ID_WORKER_QUIT;
            break;
        case 'l':
            msgid = MSG_ID_LOCK_START;
            break;
        case 'u':
            msgid = MSG_ID_UNLOCK_START;
            break;
        case '1':
            msgid = MSG_ID_SEND;
            break;
        case '2':
            msgid = MSG_ID_RECEIVE;
            break;
        }
    }
    bool ret = ASMP_mpmq_send(&(g_mptasks[index].mq), msgid, data);
    return ret;
}

bool CmdParser::app_cmd_mq_receive(int nargc, char *const *nargv, const char *options,
        const struct option *long_options)
{
    int opt;
    optind = 1;
    int option_index = 0;
    int index = 0;
    while ((opt = parse_arg_opt_long(nargc, nargv, options, long_options, &option_index)) != -1)
    {
        switch(opt)
        {
        case 'i':
            index = atoi(optarg);
            break;
        default:
            break;
        }
    }

    uint32_t data  = 0;
    int msgid = ASMP_mpmq_receive(&(g_mptasks[index].mq), &data);
    printf("Worker response: ID = %d, data = %08x\n", msgid, data);
    switch(msgid)
    {
        case MSG_ID_READ_SHM_DONE:
            printf("READ SHM DONE\n");
            break;
        case MSG_ID_WRITE_SHM_DONE:
            printf("WRITE SHM DONE\n");
            break;
        case MSG_ID_LOCK_DONE:
            printf("LOCK DONE\n");
            break;
        case MSG_ID_UNLOCK_DONE:
            printf("UNLOCK DONE\n");
            break;
        case MSG_ID_SEND_DONE:
            printf("SEND DONE\n");
            break;
        case MSG_ID_RECEIVE_DONE:
            printf("REV DONE\n");
            break;
        default:
            return false;
            break;
    }
    return true;
}

bool CmdParser::app_cmd_mq_notify(int nargc, char *const *nargv, const char *options,
        const struct option *long_options)
{
    int opt;
    optind = 1;
    int option_index = 0;
    int index = 0;
    while ((opt = parse_arg_opt_long(nargc, nargv, options, long_options, &option_index)) != -1)
    {
        switch(opt)
        {
        case 'i':
            index = atoi(optarg);
            break;
        }
    }
    //ASMP_mpmq_notify(mpmq_t *mq, int signo, void *sigdata);
    return true;
}

bool CmdParser::app_cmd_mpmutex_init(int nargc, char *const *nargv, const char *options,
        const struct option *long_options)
{
    int opt;
    optind = 1;
    int option_index = 0;
    int index = 0;
    while ((opt = parse_arg_opt_long(nargc, nargv, options, long_options, &option_index)) != -1)
    {
        switch(opt)
        {
        case 'k':
            g_mptasks[index].mutex_key = atoi(optarg);
            break;
        case 'i':
            index = atoi(optarg);
            break;
        }
    }
    bool ret = ASMP_mpmutex_init(&(g_mptasks[index].sem), g_mptasks[index].mutex_key);
    if (ret)
        printf("Mutex init succeed\n");
    else
        printf("Mutex init error\n");
    return ret;
}

bool CmdParser::app_cmd_mpmutex_destroy(int nargc, char *const *nargv, const char *options,
        const struct option *long_options)
{
    int opt;
    optind = 1;
    int option_index = 0;
    int index = 0;
    while ((opt = parse_arg_opt_long(nargc, nargv, options, long_options, &option_index)) != -1)
    {
        switch(opt)
        {
        case 'i':
            index = atoi(optarg);
            break;
        }
    }
    return ASMP_mpmutex_destroy(&(g_mptasks[index].sem));
}

bool CmdParser::app_cmd_mpmutex_lock(int nargc, char *const *nargv, const char *options,
        const struct option *long_options)
{
    int opt;
    optind = 1;
    int option_index = 0;
    int index = 0;
    while ((opt = parse_arg_opt_long(nargc, nargv, options, long_options, &option_index)) != -1)
    {
        switch(opt)
        {
        case 'i':
            index = atoi(optarg);
            break;
        }
    }
    return ASMP_mpmutex_lock(&(g_mptasks[index].sem));
}

bool CmdParser::app_cmd_mpmutex_trylock(int nargc, char *const *nargv, const char *options,
        const struct option *long_options)
{
    int opt;
    optind = 1;
    int option_index = 0;
    int index = 0;
    while ((opt = parse_arg_opt_long(nargc, nargv, options, long_options, &option_index)) != -1)
    {
        switch(opt)
        {
        case 'i':
            index = atoi(optarg);
            break;
        }
    }
    return ASMP_mpmutex_trylock(&(g_mptasks[index].sem));
}

bool CmdParser::app_cmd_mpmutex_unlock(int nargc, char *const *nargv, const char *options,
        const struct option *long_options)
{
    int opt;
    optind = 1;
    int option_index = 0;
    int index = 0;
    while ((opt = parse_arg_opt_long(nargc, nargv, options, long_options, &option_index)) != -1)
    {
        switch(opt)
        {
        case 'i':
            index = atoi(optarg);
            break;
        }
    }
    return ASMP_mpmutex_unlock(&(g_mptasks[index].sem));
}

bool CmdParser::app_cmd_mpshm_init(int nargc, char *const *nargv, const char *options,
        const struct option *long_options)
{
    int opt;
    int option_index = 0;
    int index = 0;
    optind = 1;
    while ((opt = parse_arg_opt_long(nargc, nargv, options, long_options, &option_index)) != -1)
    {
        switch(opt)
        {
        case 'i':
            index = atoi(optarg);
            break;
        case 'k':
            g_mptasks[index].shm_key = atoi(optarg);
            break;
        case 's':
            g_mptasks[index].g_shm_size = atoi(optarg);
            break;
        }
    }
    bool ret = ASMP_mpshm_init(&(g_mptasks[index].g_shm), g_mptasks[index].shm_key, g_mptasks[index].g_shm_size);
    if (ret)
        printf("SHM_INIT finished.\n");
    else
        printf("SHM_INIT error.\n");
    return ret;
}

bool CmdParser::app_cmd_mpshm_destroy(int nargc, char *const *nargv, const char *options,
        const struct option *long_options)
{
    int opt;
    optind = 1;
    int option_index = 0;
    int index  = 0;
    while ((opt = parse_arg_opt_long(nargc, nargv, options, long_options, &option_index)) != -1)
    {
        switch(opt)
        {
        case 'i':
            index = atoi(optarg);
            break;
        }
    }
    return ASMP_mpshm_destroy(&(g_mptasks[index].g_shm));
}

bool CmdParser::app_cmd_mpshm_attach(int nargc, char *const *nargv, const char *options,
        const struct option *long_options)
{
    int opt;
    optind = 1;
    int option_index = 0;
    int shmflag = 0;    
    int index  = 0;
    while ((opt = parse_arg_opt_long(nargc, nargv, options, long_options, &option_index)) != -1)
    {
        switch(opt)
        {
        case 'i':
            index = atoi(optarg);
            break;
        case 'f':
            shmflag = atoi(optarg);
            break;
        }
    }
    g_mptasks[index].g_shared = (char*)ASMP_mpshm_attach(&(g_mptasks[index].g_shm),shmflag);
    return true;
}

bool CmdParser::app_cmd_mpshm_detach(int nargc, char *const *nargv, const char *options,
        const struct option *long_options)
{
    int opt;
    optind = 1;
    int option_index = 0;
    int index  = 0;
    while ((opt = parse_arg_opt_long(nargc, nargv, options, long_options, &option_index)) != -1)
    {
        switch(opt)
        {
        case 'i':
            index = atoi(optarg);
            break;
        }
    }
    return ASMP_mpshm_detach(&(g_mptasks[index].g_shm));
}

bool CmdParser::app_cmd_mpshm_virt2phys(int nargc, char *const *nargv, const char *options,
        const struct option *long_options)
{
    int opt;
    optind = 1;
    int option_index = 0;
    int index  = 0;
    while ((opt = parse_arg_opt_long(nargc, nargv, options, long_options, &option_index)) != -1)
    {
        switch(opt)
        {
        case 'i':
            index = atoi(optarg);
            break;
        }
    }
    ASMP_mpshm_virt2phys(&(g_mptasks[index].g_shm), NULL);
    return true;
}

bool CmdParser::app_cmd_mpshm_phys2virt(int nargc, char *const *nargv, const char *options,
const struct option *long_options)
{
    int opt;
    optind = 1;
    int option_index = 0;
    int index  = 0;
    while ((opt = parse_arg_opt_long(nargc, nargv, options, long_options, &option_index)) != -1)
    {
        switch(opt)
        {
        case 'i':
            index = atoi(optarg);
            break;
        }
    }
    mpshm_phys2virt(&(g_mptasks[index].g_shm), NULL);
    return true;
}

bool CmdParser::app_cmd_mpshm_control(int nargc, char *const *nargv, const char *options,
const struct option *long_options)
{
    int opt;
    optind = 1;
    int option_index = 0;
    int cmd = 0;
    int index  = 0;
    while ((opt = parse_arg_opt_long(nargc, nargv, options, long_options, &option_index)) != -1)
    {
        switch(opt)
        {
        case 'i':
            index = atoi(optarg);
            break;
        case 'n':
            cmd = MPC_POWERON;
            break;
        case 'f':
            cmd = MPC_POWEROFF;
            break;
        case 'r':
            cmd = MPC_RETENTION;
            break;
        }
    }
    bool ret = ASMP_mpshm_control(&(g_mptasks[index].g_shm), cmd, NULL);
    if (ret)
        printf("Shm control done\n");
    else
        printf("Shm control error\n");
    return ret;
}

bool CmdParser::app_cmd_mpshm_remap(int nargc, char *const *nargv, const char *options,
const struct option *long_options)
{
    int opt;
    optind = 1;
    int option_index = 0;
    int index  = 0;
    while ((opt = parse_arg_opt_long(nargc, nargv, options, long_options, &option_index)) != -1)
    {
        switch(opt)
        {
        case 'i':
            index = atoi(optarg);
            break;
        }
    }
    return ASMP_mpshm_remap(&(g_mptasks[index].g_shm), NULL);
}

bool CmdParser::app_cmd_task_init(int nargc, char *const *nargv, const char *options,
        const struct option *long_options)
{
    int index = 0;
    char filename[128];

    int opt = -1;
    int option_index = 0;
    optind = 1;
    while ((opt = parse_arg_opt_long(nargc, nargv, options, long_options, &option_index)) != -1)
    {
        switch(opt)
        {
        case 'i':
            index = atoi(optarg);
            break;
        case 'f':
            sprintf(filename, "%s/%s", g_workerpath, optarg);
            break;
        }
    }
    //call api
    bool ret = ASMP_mptask_init(&g_mptasks[index].task, filename);
    if (ret)
        printf("Task init succeed\n");
    else
        printf("Task init error\n");
    return ret;
}

bool CmdParser::app_cmd_task_init_secure(int nargc, char *const *nargv, const char *options,
        const struct option *long_options)
{
    int index = 0;
    char filename[128];

    int opt = -1;
    int option_index = 0;
    optind = 1;
    while ((opt = parse_arg_opt_long(nargc, nargv, options, long_options, &option_index)) != -1)
    {
        switch(opt)
        {
        case 'i':
            index = atoi(optarg);
            break;
        case 'f':
            sprintf(filename, "%s/%s", g_workerpath, optarg);
            break;
        }
    }
    //call api
    bool ret = ASMP_mptask_init_secure(&g_mptasks[index].task, filename);
    if (ret)
        printf("Task init secure succeed\n");
    else
        printf("Task init secure error\n");
    return ret;
}

bool CmdParser::app_cmd_task_destroy(int nargc, char *const *nargv, const char *options,
        const struct option *long_options)
{
    int index = 0;

    int opt = -1;
    int option_index = 0;
    optind = 1;
    while ((opt = parse_arg_opt_long(nargc, nargv, options, long_options, &option_index)) != -1)
    {
        switch(opt)
        {
        case 'i':
            index = atoi(optarg);
            break;
        }
    }
    bool ret = ASMP_mptask_destroy(&g_mptasks[index].task);
    if (ret)
        printf("Task destroyed\n");
    else
        printf("Task destroy error\n");
    return ret;
}

bool CmdParser::app_cmd_task_assign(int nargc, char *const *nargv, const char *options,
        const struct option *long_options)
{
    int index = 0;

    int opt = -1;
    int option_index = 0;
    optind = 1;
    while ((opt = parse_arg_opt_long(nargc, nargv, options, long_options, &option_index)) != -1)
    {
        switch(opt)
        {
        case 'i':
            index = atoi(optarg);
            break;
        }
    }
    bool ret = ASMP_mptask_assign(&g_mptasks[index].task);
    if (ret)
    {
        g_mptasks[index].cpuid = ASMP_mptask_getcpuid(&g_mptasks[index].task);
    }
    return ret;
}

bool CmdParser::app_cmd_task_bind(int nargc, char *const *nargv, const char *options,
        const struct option *long_options)
{
    int index = 0;
    bool bind_mq = false;
    bool bind_mutex = false;
    bool bind_shm = false;
    int mq_index = 0;
    int mutex_index = 0;
    int shm_index = 0;

    int opt = -1;
    int option_index = 0;
    optind = 1;
    while ((opt = parse_arg_opt_long(nargc, nargv, options, long_options, &option_index)) != -1)
    {
        switch(opt)
        {
        case 'i':
            index = atoi(optarg);
            break;
        case 'm':
            mq_index = atoi(optarg);
            bind_mq = true;
            break;
        case 'x':
            mutex_index = atoi(optarg);
            bind_mutex = true;
            break;
        case 's':
            shm_index =  atoi(optarg);
            bind_shm = true;
            break;
        }
    }
    bool ret = true;
    if (bind_mq)
        ret = ASMP_mptask_bind(&g_mptasks[index].task, &g_mptasks[mq_index].mq.super);
    if (bind_mutex)
        ret = ASMP_mptask_bind(&g_mptasks[index].task, &g_mptasks[mutex_index].sem.super);
    if (bind_shm)
        ret = ASMP_mptask_bind(&g_mptasks[index].task, &g_mptasks[shm_index].g_shm.super);
    return ret;
}

bool CmdParser::app_cmd_task_exec(int nargc, char *const *nargv, const char *options,
        const struct option *long_options)
{
    int index = 0;

    int opt = -1;
    int option_index = 0;
    optind = 1;
    while ((opt = parse_arg_opt_long(nargc, nargv, options, long_options, &option_index)) != -1)
    {
        switch(opt)
        {
        case 'i':
            index = atoi(optarg);
            break;
        }
    }
    bool ret = ASMP_mptask_exec(&g_mptasks[index].task);
    if (ret)
        printf("Task executed\n");
    else
        printf("Task execute error\n");
    return ret;
}

bool CmdParser::app_cmd_task_join(int nargc, char *const *nargv, const char *options,
        const struct option *long_options)
{
    int index = 0;

    int opt = -1;
    int option_index = 0;
    optind = 1;
    while ((opt = parse_arg_opt_long(nargc, nargv, options, long_options, &option_index)) != -1)
    {
        switch(opt)
        {
        case 'i':
            index = atoi(optarg);
            break;
        }
    }
    return ASMP_mptask_join(&g_mptasks[index].task);
}

bool CmdParser::app_cmd_task_attr_init(int nargc, char *const *nargv, const char *options,
        const struct option *long_options)
{
    int index = 0;

    int opt = -1;
    int option_index = 0;
    optind = 1;
    while ((opt = parse_arg_opt_long(nargc, nargv, options, long_options, &option_index)) != -1)
    {
        switch(opt)
        {
        case 'i':
            index = atoi(optarg);
            break;
        }
    }
    bool ret = ASMP_mptask_attr_init(&g_mptasks[index].attr);
    return ret;
}

bool CmdParser::app_cmd_task_setattr(int nargc, char *const *nargv, const char *options,
        const struct option *long_options)
{
    int index = 0;

    int opt = -1;
    int option_index = 0;
    optind = 1;
    while ((opt = parse_arg_opt_long(nargc, nargv, options, long_options, &option_index)) != -1)
    {
        switch(opt)
        {
        case 'i':
            index = atoi(optarg);
            break;
        }
    }
    bool ret = ASMP_mptask_setattr(&g_mptasks[index].task, &g_mptasks[index].attr);
    return ret;
}

bool CmdParser::app_cmd_task_getattr(int nargc, char *const *nargv, const char *options,
        const struct option *long_options)
{
    int index = 0;

    int opt = -1;
    int option_index = 0;
    optind = 1;
    while ((opt = parse_arg_opt_long(nargc, nargv, options, long_options, &option_index)) != -1)
    {
        switch(opt)
        {
        case 'i':
            index = atoi(optarg);
            break;
        }
    }
    bool ret = ASMP_mptask_getattr(&g_mptasks[index].task, &g_mptasks[index].attr);
    return ret;
}

bool CmdParser::app_cmd_write_shm(int nargc, char *const *nargv, const char *options,
        const struct option *long_options)
{
    int opt;
    optind = 1;
    int option_index = 0;
    int index  = 0;
    char* pValue = NULL;
    while ((opt = parse_arg_opt_long(nargc, nargv, options, long_options, &option_index)) != -1)
    {
        switch(opt)
        {
        case 'i':
            index = atoi(optarg);
            break;
        case 'v':
            pValue = optarg;
            break;
        }
    }

    if (pValue != NULL)
    {
        strncpy(g_mptasks[index].g_shared, pValue ,g_mptasks[index].g_shm_size);
        return true;
    }
    else
    {
        memset(g_mptasks[index].g_shared, 0 ,g_mptasks[index].g_shm_size);
        return true;
    }
}

bool CmdParser::app_cmd_read_shm(int nargc, char *const *nargv, const char *options,
        const struct option *long_options)
{
    int opt;
    optind = 1;
    int option_index = 0;
    int index  = 0;
    while ((opt = parse_arg_opt_long(nargc, nargv, options, long_options, &option_index)) != -1)
    {
        switch(opt)
        {
        case 'i':
            index = atoi(optarg);
            break;
        }
    }
    printf("%s\n", g_mptasks[index].g_shared);
    return true;
}

bool CmdParser::app_cmd_quit(int nargc, char *const *nargv, const char *options,
        const struct option *long_options)
{
    return true;
}

