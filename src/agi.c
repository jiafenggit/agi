/*
 * Author: Romario Maxwell
 *
 * Operations to work with the Asterisk Gateway Interface (AGI) protocol
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>

#include <sys/types.h>
#include <sys/msg.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/un.h>

#include <netdb.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include "agi.h"
#include "utils.h"
#include "log.h"
#include "string.h"     /* strlcpy */

#define strcmp6id(s, c0, c1, c2, c3, c4, c5, c6, c7)                      \
    s[0] == c0 && s[1] == c1 && s[2] == c2 && s[3] == c3 && s[4] == c4        \
    && s[5] == c5

#define strcmp2callingt(s, c0, c1, c2, c3, c4, c5, c6, c7, c8, c9)        \
    s[8] == c8  && s[9] == c9

#define strcmp4calling(s, c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10)    \
    s[7] == c7 && s[8] == c8 && s[9] == c9 && s[10] == c10


static void struct_member_helper(agi_environment_t *e,
    const char *variable, size_t variable_len,
    const char *value, size_t value_len);

static void
struct_member_helper(agi_environment_t *e,
    const char *variable, size_t variable_len
    const char *value, size_t value_len)
{
    int         n;  /* argument number */
    const char *c;

    switch (variable_len) {
    case 4:
        if (strcmp4(variable, 't', 'y', 'p', 'e')) {
            e->type = (char *)value;
            break;
        }
        
        if (strcmp4(variable, 'd', 'n', 'i', 'd')) {
            e->dnid = (char *)value;
            break;
        }

        break;

    case 5:
        if (strcmp5(variable, 'r', 'd', 'n', 'i', 's')) {
            e->rdnis = (char *)value;
            break;
        }

        if (strcmp4(variable, 'a', 'r', 'g', '_')) {
            c = variable + (sizeof "arg_" - 1);     /* argument number */

            if (c[0] >= '1' && c[0] <= '9') {
                n = c[0] - '1';     /* avoid extra n - 1 sub for index below */
                e->argv[n] = (char *)value;
            }
            else
                /* invalid argument number (valid: 1 - 9) */
                return -1;

            break;
        }

        break;

    case 6:
        if (strcmp4(variable, 'a', 'r', 'g', '_')) {
            c = variable + (sizeof "arg_" - 1);     /* argument number */

            if (c[0] >= '1' && c[0] <= '9' && c[1] >= '0' && c[1] <= '9') {
                n = (c[0] - '0') * 10;
                n += c[1] - '0';

                e->argv[n - 1] = (char *)value;
            }
            else
                /* invalid argument number (valid: 10 - 99) */
                return -1;
        }

        break;

    case 7:
        if (strcmp7(variable, 'n', 'e', 't', 'w', 'o', 'r', 'k')) {
            e->network = (char *)value;
            e->network_n =
                variable_len == 3 && strcmp3(e->network, 'y', 'e', 's');
            break;
        }

        if (strcmp7(variable, 'r', 'e', 'q', 'u', 'e', 's', 't')) {
            e->request = (char *)value;
            break;
        }

        if (strcmp7(variable, 'c', 'h', 'a', 'n', 'n', 'e', 'l')) {
            e->channel = (char *)value;
            break;
        }

        if (strcmp7(variable, 'v', 'e', 'r', 's', 'i', 'o', 'n')) {
            e->version = (char *)value;
            break;
        }

        if (strcmp7(variable, 'c', 'o', 'n', 't', 'e', 'x', 't')) {
            e->context = (char *)value;
            break;
        }

        /* TODO: refactor and remove code duplication */
        if (strcmp4(variable, 'a', 'r', 'g', '_')) {
            c = variable + (sizeof "arg_" - 1);     /* argument number */

            if (c[0] == '1') {
                if ((c[1] == '0' || c[1] == '1')
                    && c[2] >= '0' && c[2] <= '9') {

                    n = (c[0] - '0') * 100;
                    n += (c[1] - '0') * 10;
                    n += c[2] - '0';

                    e->argv[n - 1] = (char *)value;
                }
                else if (c[1] == '2' && c[2] >= '0' && c[2] <= '7') {
                    n = (c[0] - '0') * 100;
                    n += (c[1] - '0') * 10;
                    n += c[2] - '0';

                    e->argv[n - 1] = (char *)value;
                }
                else
                    /* invalid argument number (valid: 100 - 127) */
                    return -1;
            }
            else
                /* invalid argument number (valid: 100 - 127) */
                return -1;

            break;
        }

        break;

    case 8:
        if (variable[6] == 'i' && variable[7] == 'd') {
            if (strcmp6id(variable,
                    'u', 'n', 'i', 'q', 'u', 'e', 'i', 'd'))
            {
                e->uniqueid = (char *)value;
                break;
            }

            if (strcmp6id(variable,
                    'c', 'a', 'l', 'l', 'e', 'r', 'i', 'd'))
            {
                e->callerid = value;
                break;
            }

            if (strcmp6id(variable,
                    't', 'h', 'r', 'e', 'a', 'd', 'i', 'd'))
            {
                e->threadid = (char *)value;
                e->threadid_n = strtol(e->threadid, (char **)NULL, 10);
                break;
            }
        } else {
            if (strcmp8(variable, 'l', 'a', 'n', 'g', 'u', 'a', 'g', 'e'))
            {
                e->language = (char *)value;
                break;
            }

            if (strcmp8(variable, 'p', 'r', 'i', 'o', 'r', 'i', 't', 'y'))
            {
                e->priority = (char *)value;
                e->priority_n = atoi(e->priority);
                break;
            }

            if (strcmp8(variable, 'e', 'n', 'h', 'a', 'n', 'c', 'e', 'd'))
            {
                e->enhanced = (char *)value;
                e->enhanced_n = variable_len == 3
                    && strcmp3(e->enhanced, '1', '.', '0');
                break;
            }
        }

        break;

    case 9:
        if (strcmp9(variable, 'e', 'x', 't', 'e', 'n', 's', 'i', 'o', 'n'))
            e->extension = (char *)value;

        break;

    case 10:
        if (strcmp8(variable, 'c', 'a', 'l', 'l', 'i', 'n', 'g', 't')) {
            if (strcmp2callingt(variable,
                    'c', 'a', 'l', 'l', 'i', 'n', 'g', 't', 'o', 'n'))
            {
                e->callington = (char *)value;
                break;
            }

            if (strcmp2callingt(variable,
                    'c', 'a', 'l', 'l', 'i', 'n', 'g', 't', 'n', 's'))
            {
                e->callingtns = (char *)value;
                break;
            }
        }

        break;

    case 11:
        if (strcmp7(variable, 'c', 'a', 'l', 'l', 'i', 'n', 'g')) {
            if (strcmp4calling(variable,
                    'c', 'a', 'l', 'l', 'i', 'n', 'g', 'p', 'r', 'e', 's'))
            {
                e->callingpres = (char *)value;
                break;
            }

            if (strcmp4calling(variable,
                    'c', 'a', 'l', 'l', 'i', 'n', 'g', 'a', 'n', 'i', '2'))
            {
                e->callingani2 = (char *)value;
                break;
            }
        } else {
            if (strcmp11(variable,
                    'a', 'c', 'c', 'o', 'u', 'n', 't', 'c', 'o', 'd', 'e'))
            {
                e->accountcode = (char *)value;
                break;
            }
        }

        break;

    case 12:
        if (strcmp12(variable,
                'c', 'a', 'l', 'l', 'e', 'r', 'i', 'd', 'n', 'a', 'm', 'e'))
        {
            e->calleridname = (char *)value;
        }

        break;

    case 14:
        if (strcmp14(variable,
                'n', 'e', 't', 'w', 'o', 'r', 'k',
                '_',
                's', 'c', 'r', 'i', 'p', 't'))
        {
            e->network_script = (char *)value;
        }

        break;

    default:
        break;
    }
}

int
agi_getenvironment(int fd, char *buf, size_t bufsz)
{
    int             rv;
    size_t          buflen = 0;
    size_t          datalen = 0;    /* agi environment length */
    ssize_t         bytes = 0;
    fd_set          master;
    fd_set          readfds;    /* temp file descriptor list for select() */
    struct timeval  tv = {1L, 500000L};

    FD_ZERO(&master);
    FD_ZERO(&readfds);

    FD_SET(fd, &master);

    /* to allow for the terminating null-character to be appended */
    buflen = bufsz - 1;

    *buf = '\0';

    while (buflen) {
        /*
         * select() on Linux modifies the struct to reflect the amount of
         * time not slept as well as when it is interrupted by a signal handler
         */
        tv.tv_sec = 1L;
        tv.tv_usec = 500000L;

        /* select() modifies the set */
        readfds = master;

        rv = select(fd + 1, &readfds, (fd_set *)NULL, (fd_set *)NULL, &tv);

        log_debug0("select() on socket ready");

        if (rv == -1) {
            if (errno != EINTR) {
                log(LOG_ERR, "select() failed");
                return -1;
            }
        }
        else if (rv == 0) {
            log(LOG_ERR, "read timeout occurred after 1.5 sec");
            break;
        }
        else {
            if (FD_ISSET(fd, &master)) {
                bytes = recv(fd, buf + datalen, buflen, 0);

                if (bytes == (ssize_t)-1) {
                    log(LOG_ERR, "recv() failed");
                    return -1;
                }

                if (bytes == (ssize_t)0) {
                    log(LOG_ERR, "remote side closed their endpoint");
                    break;
                }

                if (datalen > (size_t)1) {
                    /* end of environment variables */
                    if ((buf[datalen] == '\n') && (buf[datalen - 1] == '\n'))
                        break;
                }

                buflen = bufsz - (size_t)bytes;
                datalen += (size_t)bytes;
            }
        }
    }

    buf[datalen + 1] = '\0';

    return 0;
}

/* Send AGI command to Asterisk */
int
agi_send_command(int fd, const char *command, char *result, char *data)
{
    int     rv;
    char    response_line[BUFSIZ];
    size_t  len;
    ssize_t bytes;

    if (send(fd, command, strlen(command)) == -1) {
        /* remote socket closed */
        if (EPIPE == errno)
            log(LOG_ERR,
                    "send() failed: Asterisk closed"
                    " its endpoint and may have died");

        log(LOG_ERR, "send() failed");

        return -1;
    }

    bytes = recv(fd, response_line, sizeof response_line, 0);
    if (bytes <= (ssize_t)0) {
        /* remote socket has been closed */
        if ((ssize_t)0 == bytes)
            log(LOG_ERR,
                    "recv() failed: Asterisk closed"
                    " its endpoint and may have died");

        log(LOG_ERR, "recv() failed");

        return -1;
    }

    response_line[bytes] = '\0';

    log_debug1("agi command response line: %s", response_line);

    rv = agi_parse_command_response_line(response_line, result, data);

    if (rv == -1)
        return -1;

    log_debug2("\n"
                   "agi command parsed result: %s\n"
                   "agi command parsed data: %s\n",
                   strlen(result) ? result : "(null)",
                   strlen(data) ? data : "(null)");

    return 0;
}

void
agi_process_environment(agi_environment_t *e, char *buf)
{
    int     rv;
    size_t  var_len, val_len;
    char    *variable, *value;

    for (;;) {
        rv = agi_parse_environment_variable_line(e, buf);

        /*
         * to determine when we reach the blank line indicating the end of the
         * AGI environment list
         */
        if (rv == 3)
            break;

        if (rv == 0) {
            var_len = e->variable_end - e->variable_start;
            variable = e->variable_start;
            variable[var_len] = '\0';

            val_len = e->value_end - e->value_start;
            value = e->value_start;
            value[val_len] = '\0';

            log_debug2("agi env line: %s: %s", variable, value);

            struct_member_helper(e, variable, var_len, value, val_len);
        }

        buf = e->value_end + 1;
    }
}
