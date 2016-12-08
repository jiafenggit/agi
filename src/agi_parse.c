/*
 * Author: Romario Maxwell
 */

#include "utils.h"
#include "agi.h"        /* agi_environment_t */
#include "string.h"

#define LF '\n'

int
agi_parse_environment_variable_line(agi_environment_t *e,
    const char *buf)
{
    unsigned char c, ch;
    const char *p;
    enum {
        sw_start = 0,
        sw_agi_a,
        sw_agi_ag,
        sw_agi_agi,
        sw_underscore_before_variable,
        sw_variable,
        sw_space_before_value,
        sw_value,
        sw_space_after_value
    } state;

    /* the last '\0' is not needed because string is zero terminated */
    static unsigned char lowcase[] =
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0\0\0\0\0\0-\0\0" "0123456789\0\0\0\0\0\0"
        "\0abcdefghijklmnopqrstuvwxyz\0\0\0\0\0"
        "\0abcdefghijklmnopqrstuvwxyz\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";

    state = sw_start;

    for (p = buf; p; p++) {
        ch = *p;

        switch (state) {
            /* first char */
            case sw_start:
                switch (ch) {
                    case 'a':
                        state = sw_agi_a;
                        break;

                    /* TODO: remove magic number and handle properly */
                    case LF:
                        return 3;

                    default:
                        return -1;
                }
                break;

            case sw_agi_a:
                switch (ch) {
                    case 'g':
                        state = sw_agi_ag;
                        break;

                    default:
                        return -1;
                }
                break;

            case sw_agi_ag:
                switch (ch) {
                    case 'i':
                        state = sw_agi_agi;
                        break;

                    default:
                        return -1;
                }
                break;

            case sw_agi_agi:
                switch (ch) {
                    case '_':
                        state = sw_underscore_before_variable;
                        break;

                    default:
                        return -1;
                }
                break;

            /* TODO */
            case sw_underscore_before_variable:
                switch (ch) {
                    case LF:    /* fall through */
                    case '\0':
                        return -1;

                    default:
                        e->variable_start = (char *)p;
                        state = sw_variable;
                        break;
                }

                break;

            case sw_variable:
                c = lowcase[ch];

                if (c)
                    break;

                if (ch == ':') {
                    e->variable_end = (char *)p;
                    state = sw_space_before_value;
                    break;
                }

                if (ch == LF) {
                    e->variable_end = (char *)p;
                    e->value_start = (char *)p;
                    e->value_end = (char *)p;
                    goto done;
                }

                if (ch == '\0')
                    return -1;

                break;

            case sw_space_before_value:
                switch (ch) {
                    case ' ':
                        break;

                    case LF:
                        e->value_start = (char *)p;
                        e->value_end = (char *)p;
                        goto done;

                    case '\0':
                        return -1;

                    default:
                        e->value_start = (char *)p;
                        state = sw_value;
                        break;
                }
                break;

            case sw_value:
                switch (ch) {
                    case ' ':
                        e->value_end = (char *)p;
                        state = sw_space_after_value;
                        break;

                    case LF:
                        e->value_end = (char *)p;
                        goto done;

                    case '\0':
                        return -1;
                }
                break;

            /* space before end of header line */
            case sw_space_after_value:
                switch (ch) {
                    case ' ':
                        break;

                    case LF:
                        goto done;

                    case '\0':
                        return -1;
                    
                    default:
                        state = sw_value;
                        break;
                }
                break;
        }
    }

done:

    return 0;
}

int
agi_parse_command_response_line(char *buf, char *result, char *data)
{
    char            *p;
    char            *res_start, *res_end, *data_start, *data_end;
    size_t          len;
    unsigned char   c, ch;
    enum {
        sw_start = 0,
        sw_code_2,
        sw_code_20,
        sw_code_200,
        sw_space_before_result,
        sw_result_r,
        sw_result_re,
        sw_result_res,
        sw_result_resu,
        sw_result_resul,
        sw_result_result,
        sw_equal_sign_after_result,
        sw_return_value,
        sw_space_before_data,
        sw_data
    } state;

    /* the last '\0' is not needed because string is zero terminated */
    static unsigned char digit[] =
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0\0\0\0\0\0-\0\0" "0123456789\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";

    state = sw_start;

    for (p = buf; p; p++) {
        ch = *p;

        switch (state) {
            /* first char */
            case sw_start:
                switch (ch) {
                    case '2':
                        state = sw_code_2;
                        break;

                    default:
                        return -1;
                }
                break;

            case sw_code_2:
                switch (ch) {
                    case '0':
                        state = sw_code_20;
                        break;

                    default:
                        return -1;
                }
                break;

            case sw_code_20:
                switch (ch) {
                    case '0':
                        state = sw_code_200;
                        break;

                    default:
                        return -1;
                }
                break;

            case sw_code_200:
                switch (ch) {
                    case ' ':
                        state = sw_space_before_result;
                        break;

                    default:
                        return -1;
                }
                break;

            case sw_space_before_result:
                switch (ch) {
                    case 'r':
                        state = sw_result_r;
                        break;

                    default:
                        return -1;
                }
                break;

            case sw_result_r:
                switch (ch) {
                    case 'e':
                        state = sw_result_re;
                        break;

                    default:
                        return -1;
                }
                break;

            case sw_result_re:
                switch (ch) {
                    case 's':
                        state = sw_result_res;
                        break;

                    default:
                        return -1;
                }
                break;

            case sw_result_res:
                switch (ch) {
                    case 'u':
                        state = sw_result_resu;
                        break;

                    default:
                        return -1;
                }
                break;

            case sw_result_resu:
                switch (ch) {
                    case 'l':
                        state = sw_result_resul;
                        break;

                    default:
                        return -1;
                }
                break;

            case sw_result_resul:
                switch (ch) {
                    case 't':
                        state = sw_result_result;
                        break;

                    default:
                        return -1;
                }
                break;

            case sw_result_result:
                switch (ch) {
                    case '=':
                        state = sw_equal_sign_after_result;
                        break;

                    default:
                        return -1;
                }
                break;

            case sw_equal_sign_after_result:
                c = digit[ch];

                if (c) {
                    res_start = p;
                    state = sw_return_value;
                    break;
                }
                else
                    return -1;

                break;

            case sw_return_value:
                c = digit[ch];

                if (c)
                    break;

                switch (ch) {
                    case LF:
                        /*
                         * 200 result=-
                         * negative value with no digit
                         */
                        if (*(p - 1) == '-')
                            return -1;

                        res_end = p;
                        data_start = p;
                        data_end = p;
                        goto done;

                    case ' ':
                        res_end = p;
                        state = sw_space_before_data;
                        break;

                    default:
                        return -1;
                }
                break;

            case sw_space_before_data:
                switch (ch) {
                    case ' ':
                        break;

                    case '\0':
                        return -1;

                    default:
                        data_start = p;
                        state = sw_data;
                        break;
                }
                break;

            case sw_data:
                switch (ch) {
                    case LF:
                        data_end = p;
                        goto done;

                    case '\0':
                        return -1;

                    default:
                        break;
                }
                break;
        }
    }

done:

    len = res_end - res_start;
    res_start[len] = '\0';
    (void)strlcpy(result, res_start, BUFSIZ);

    len = data_end - data_start;
    data_start[len] = '\0';
    (void)strlcpy(data, data_start, BUFSIZ);

    return 0;
}
