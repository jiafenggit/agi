/*
 * Author: Romario Maxwell
 */

#include <stdio.h>          /* BUFSIZ, snprintf */
#include <stdlib.h>
#include <string.h>         /* memcpy, strlen */

#include <sys/types.h>      /* off_t */

#include <linux/limits.h>   /* PATH_MAX */

#include "agi.h"
#include "string.h"     /* strlcpy */
#include "utils.h"      /* AST_XXX */

#define LF '\n'

/*
 * Reference the line "#define AGI_BUF_LEN 2048" in res/res_agi.c in the
 * Asterisk project
 */
#define AGI_BUF_LEN 2048

#define CMD_GET_DATA_LEN (sizeof "get data" - 1) + 1 + PATH_MAX + 1           \
    + INT32_LEN + 1 + INT32_LEN

#define CMD_HANGUP_LEN (sizeof "hangup" - 1) + 1 + AST_CHANNEL_NAME

#define CMD_CHANNEL_STATUS_LEN (sizeof "channel status" - 1) + 1              \
    + AST_CHANNEL_NAME

#define CMD_SET_CONTEXT_LEN (sizeof "set context" - 1) + 1 + AST_MAX_CONTEXT

#define CMD_SET_EXTENSION_LEN (sizeof "set extension" - 1) + 1                \
    + AST_MAX_EXTENSION

int
agi_command_exec(int fd, const char *application, const char *options)
{
    int     result_n;
    char    result[BUFSIZ];
    char    data[BUFSIZ];
    char    command[AGI_BUF_LEN];

    (void)snprintf(command, sizeof command,
                   "exec %s %s" LF, application, options);

    (void)agi_send_command(fd, command, result, data);

    result_n = atoi(result);

    return result_n;
}

int
agi_command_getdata(int fd, const char *prompt, char *s, int maxlen,
    int timeout)
{
    int     result_n;
    char    result[BUFSIZ];
    char    data[BUFSIZ];
    char    command[CMD_GET_DATA_LEN + 1];

    (void)snprintf(command, sizeof command,
                   "get data %s %d %d" LF, prompt, timeout, maxlen);

    (void)agi_send_command(fd, command, result, data);

    result_n = atoi(result);
    if (result_n != -1)
        (void)strlcpy(s, result, 256); /* TODO: Remove magic number! */

    return result_n;
}

int
agi_command_getfullvariable(int fd, char *buf, const char *name,
    const char *chan)
{
    int     result_n;
    char    result[BUFSIZ];
    char    data[BUFSIZ];
    char    command[AGI_BUF_LEN];

    (void)snprintf(command, sizeof command,
                   "get full variable %s %s" LF, name, chan);

    (void)agi_send_command(fd, command, result, data);

    result_n = atoi(result);

    if (result_n == 1)
        /* remove parentheses around variable value */
        (void)memcpy(buf, data + 1, strlen(data + 1) - 1);

    return result_n;
}

int
agi_command_streamfile(int fd, const char *filename,
    const char *escapedigits, off_t sample_offset)
{
    int     result_n;
    char    result[BUFSIZ];
    char    data[BUFSIZ];
    char    command[AGI_BUF_LEN];

    (void)snprintf(command, sizeof command,
                   "stream file %s %s" LF, filename, escapedigits);

    (void)agi_send_command(fd, command, result, data);

    result_n = atoi(result);

    return result_n;
}

int
agi_command_answer(int fd)
{
    int     result_n;
    char    result[BUFSIZ];
    char    data[BUFSIZ];

    (void)agi_send_command(fd, "answer" LF, result, data);

    result_n = atoi(result);

    return result_n;
}

int
agi_command_noop(int fd)
{
    int     result_n;
    char    result[BUFSIZ];
    char    data[BUFSIZ];

    (void)agi_send_command(fd, "noop" LF, result, data);

    result_n = atoi(result);

    return result_n;
}

int
agi_command_hangup(int fd, const char *channelname)
{
    int     result_n;
    char    result[BUFSIZ];
    char    data[BUFSIZ];
    char    command[CMD_HANGUP_LEN + 1];

    if (channelname) {
        (void)snprintf(command, sizeof command, "hangup %s" LF, channelname);
        (void)agi_send_command(fd, command, result, data);
    }
    else
        (void)agi_send_command(fd, "hangup" LF, result, data);

    result_n = atoi(result);

    return result_n;
}

int
agi_command_asyncagibreak(int fd)
{
    int     result_n;
    char    result[BUFSIZ];
    char    data[BUFSIZ];

    (void)agi_send_command(fd, "asyncagi break" LF, result, data);

    result_n = atoi(result);

    return result_n;
}

int
agi_command_channelstatus(int fd, const char *channelname)
{
    int     result_n;
    char    result[BUFSIZ];
    char    data[BUFSIZ];
    char    command[CMD_CHANNEL_STATUS_LEN + 1];

    if (channelname) {
        (void)snprintf(command, sizeof command,
                       "channel status %s" LF, channelname);
        
        (void)agi_send_command(fd, command, result, data);
    }
    else
        (void)agi_send_command(fd, "channel status" LF, result, data);

    result_n = atoi(result);

    return result_n;
}

int
agi_command_databasedel(int fd)
{
    return -1;
}

int
agi_command_databasedeltree(int fd)
{
    return -1;
}

int
agi_command_databaseget(int fd)
{
    return -1;
}

int
agi_command_databaseput(int fd)
{
    return -1;
}

int
agi_command_getoption(int fd)
{
    return -1;
}

int
agi_command_getvariable(int fd, char *buf, const char *variablename)
{
    int     result_n;
    char    result[BUFSIZ];
    char    data[BUFSIZ];
    char    command[AGI_BUF_LEN];

    (void)snprintf(command, sizeof command, "get variable %s" LF, variablename);

    (void)agi_send_command(fd, command, result, data);

    result_n = atoi(result);

    if (result_n == 1)
        /* remove parentheses around variable value */
        (void)memcpy(buf, data + 1, strlen(data + 1) - 1);

    return result_n;
}

int
agi_command_receivechar(int fd, unsigned long timeout)
{
    int     result_n;
    char    result[BUFSIZ];
    char    data[BUFSIZ];
    char    command[AGI_BUF_LEN];

    (void)snprintf(command, sizeof command, "receive char %lu" LF, timeout);

    (void)agi_send_command(fd, command, result, data);

    result_n = atoi(result);

    return result_n;
}

int
agi_command_receivetext(int fd)
{
    return -1;
}

int
agi_command_recordfile(int fd)
{
    return -1;
}

int
agi_command_sayalpha(int fd)
{
    return -1;
}

int
agi_command_saydigits(int fd, const char *number,
    const char *escape_digits)
{
    int     result_n;
    char    result[BUFSIZ];
    char    data[BUFSIZ];
    char    command[AGI_BUF_LEN];

    (void)snprintf(command, sizeof command,
                   "say digits %s %s" LF, number, escape_digits);

    (void)agi_send_command(fd, command, result, data);

    result_n = atoi(result);

    return result_n;
}

int
agi_command_saynumber(int fd)
{
    return -1;
}

int
agi_command_sayphonetic(int fd)
{
    return -1;
}

int
agi_command_saydate(int fd, unsigned long date, const char *escape_digits)
{
    int     result_n;
    char    result[BUFSIZ];
    char    data[BUFSIZ];
    char    command[AGI_BUF_LEN];

    (void)snprintf(command, sizeof command,
                   "say data %lu %s" LF, date, escape_digits);

    (void)agi_send_command(fd, command, result, data);

    result_n = atoi(result);

    return result_n;
}

int
agi_command_saytime(int fd, unsigned long time, const char *escapedigits)
{
    int     result_n;
    char    result[BUFSIZ];
    char    data[BUFSIZ];
    char    command[AGI_BUF_LEN];

    (void)snprintf(command, sizeof command,
                   "say time %lu %s" LF, time, escapedigits);

    (void)agi_send_command(fd, command, result, data);

    result_n = atoi(result);

    return result_n;
}

int
agi_command_saydatetime(int fd)
{
    return -1;
}

int
agi_command_sendimage(int fd)
{
    return -1;
}

int
agi_command_sendtext(int fd)
{
    return -1;
}

int
agi_command_setautohangup(int fd, unsigned long time)
{
    int     result_n;
    char    result[BUFSIZ];
    char    data[BUFSIZ];
    char    command[AGI_BUF_LEN];

    (void)snprintf(command, sizeof command, "set autohangup %lu" LF, time);

    (void)agi_send_command(fd, command, result, data);

    result_n = atoi(result);

    return result_n;
}

int
agi_command_setcallerid(int fd, const char *number)
{
    int     result_n;
    char    result[BUFSIZ];
    char    data[BUFSIZ];
    char    command[AGI_BUF_LEN];

    (void)snprintf(command, sizeof command, "set callerid %s" LF, number);

    (void)agi_send_command(fd, command, result, data);

    result_n = atoi(result);

    return result_n;
}

int
agi_command_setcontext(int fd, const char *context)
{
    int     result_n;
    char    result[BUFSIZ];
    char    data[BUFSIZ];
    char    command[CMD_SET_CONTEXT_LEN + 1];

    (void)snprintf(command, sizeof command, "set context %s" LF, context);

    (void)agi_send_command(fd, command, result, data);

    result_n = atoi(result);

    return result_n;
}

int
agi_command_setextension(int fd, const char *extension)
{
    int     result_n;
    char    result[BUFSIZ];
    char    data[BUFSIZ];
    char    command[CMD_SET_EXTENSION_LEN + 1];

    (void)snprintf(command, sizeof command, "set extension %s" LF, extension);

    (void)agi_send_command(fd, command, result, data);

    result_n = atoi(result);

    return result_n;
}

int
agi_command_setmusic(int fd)
{
    return -1;
}

int
agi_command_setpriority(int fd, const char *priority)
{
    int     result_n;
    char    result[BUFSIZ];
    char    data[BUFSIZ];
    char    command[AGI_BUF_LEN];

    (void)snprintf(command, sizeof command, "set priority %s" LF, priority);

    (void)agi_send_command(fd, command, result, data);

    result_n = atoi(result);

    return result_n;
}

int
agi_command_setvariable(int fd, const char *name, const char *value)
{
    int     result_n;
    char    result[BUFSIZ];
    char    data[BUFSIZ];
    char    command[AGI_BUF_LEN];

    (void)snprintf(command, sizeof command,
                   "set variable %s %s" LF, name, value);

    (void)agi_send_command(fd, command, result, data);

    result_n = atoi(result);

    return result_n;
}

int
agi_command_controlstreamfile(int fd)
{
    return -1;
}

int
agi_command_tddmode(int fd, int boolean)
{
    int     result_n;
    char    result[BUFSIZ];
    char    data[BUFSIZ];

    if (boolean)
        (void)agi_send_command(fd, "tdd mode on" LF, result, data);
    else
        (void)agi_send_command(fd, "tdd mode off" LF, result, data);

    result_n = atoi(result);

    return result_n;
}

int
agi_command_verbose(int fd, const char *message, int level)
{
    int     result_n;
    char    result[BUFSIZ];
    char    data[BUFSIZ];
    char    command[AGI_BUF_LEN];

    (void)snprintf(command, sizeof command, "verbose %s %d" LF, message, level);

    (void)agi_send_command(fd, command, result, data);

    result_n = atoi(result);

    return result_n;
}

int
agi_command_waitfordigit(int fd)
{
    return -1;
}

int
agi_command_speechcreate(int fd)
{
    return -1;
}

int
agi_command_speechset(int fd, const char *name, const char *value)
{
    int     result_n;
    char    result[BUFSIZ];
    char    data[BUFSIZ];
    char    command[AGI_BUF_LEN];

    (void)snprintf(command, sizeof command, "speech set %s %s" LF, name, value);

    (void)agi_send_command(fd, command, result, data);

    result_n = atoi(result);

    return result_n;
}

int
agi_command_speechdestroy(int fd)
{
    int     result_n;
    char    result[BUFSIZ];
    char    data[BUFSIZ];

    (void)agi_send_command(fd, "speech destroy" LF, result, data);

    result_n = atoi(result);

    return result_n;
}

int
agi_command_speechloadgrammar(int fd)
{
    return -1;
}

int
agi_command_speechunloadgrammar(int fd)
{
    return -1;
}

int
agi_command_speechactivategrammar(int fd)
{
    return -1;
}

int
agi_command_speechdeactivategrammar(int fd)
{
    return -1;
}

int
agi_command_speechrecognize(int fd)
{
    return -1;
}

int
agi_command_gosub(int fd)
{
    return -1;
}
