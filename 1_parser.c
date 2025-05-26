/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   1_parser.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lrandria <lrandria@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/05 13:40:39 by lrandria          #+#    #+#             */
/*   Updated: 2025/05/26 19:02:56 by lrandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_traceroute.h"

static int check_value(const char *str, int flag) {

    char *endptr;
    long val;

    val = strtol(str, &endptr, 10);
    if (endptr == str || *endptr != '\0')
        oops_crash(E_BAD_ARG, str);
    if (flag == OPT_FIRST_HOP && (val <= 0 || val > 30))
        oops_crash(E_BAD_FIRSTHOP, NULL);
    else if (flag == OPT_MAX_HOP) {
        if (val == 0)
            oops_crash(E_BAD_FIRSTHOP, NULL);
        else if (val > 255)
            oops_crash(E_BAD_MAXHOP, NULL);
    }
    else if (flag == OPT_NB_PROBES && (val <= 0 || val > MAX_PROBES))
        oops_crash(E_BAD_PROB, NULL);
    else if (flag == OPT_PORT && (val <= 0 || val > 65536))
        oops_crash(E_BAD_PORT, NULL);
    else if (flag == OPT_SENDWAIT && val <= 0)
        oops_crash(E_BAD_SENDWAIT, str);
    return (uint16_t)val;
}

static void set_option(t_parser *options, int flag, char *arg) {
    
    uint16_t val;

    options->flags |= flag;
    if (arg) {
        val = check_value(arg, flag);
        if (flag == OPT_FIRST_HOP)
            options->first_hop = val;
        else if (flag == OPT_MAX_HOP)
            options->max_hop = val;
        else if (flag == OPT_NB_PROBES)
            options->nb_probes = val;
        else if (flag == OPT_PORT)
            options->port = val;
        else if (flag == OPT_SENDWAIT)
            options->send_wait = val;
    }
}

void parse_args(int argc, char *argv[], t_parser *args) {

    int found_dest = 0;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (strcmp(argv[i], "--help") == 0) {
                print_help();
                exit(EXIT_SUCCESS);
            }
            else if (strcmp(argv[i], "-f") == 0) {
                if (argv[i + 1] == NULL)
                    oops_crash(E_MISSING_ARG, E_TRY_HELP);
                set_option(args, OPT_FIRST_HOP, argv[++i]);
            }
            else if (strcmp(argv[i], "-m") == 0) {
                if (argv[i + 1] == NULL)
                    oops_crash(E_MISSING_ARG, E_TRY_HELP);
                set_option(args, OPT_MAX_HOP, argv[++i]);
            }
            else if (strcmp(argv[i], "-p") == 0) {
                if (argv[i + 1] == NULL)
                    oops_crash(E_MISSING_ARG, E_TRY_HELP);
                set_option(args, OPT_PORT, argv[++i]);
            }
            else if (strcmp(argv[i], "-q") == 0)  {
                if (argv[i + 1] == NULL)
                    oops_crash(E_MISSING_ARG, E_TRY_HELP);
                set_option(args, OPT_NB_PROBES, argv[++i]); 
            }
            else if (strcmp(argv[i], "-z") == 0) {
                if (argv[i + 1] == NULL)
                    oops_crash(E_MISSING_ARG, E_TRY_HELP);
                set_option(args, OPT_SENDWAIT, argv[++i]);
            }
        }
        else {
            if (found_dest)
                oops_crash(E_MAX_DEST, E_TRY_HELP);
            found_dest = 1;
            args->dest = argv[i];
        }
    }
    if (found_dest == 0)
        oops_crash(E_MISSING_DEST, E_TRY_HELP);
}