/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   1_parser.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lrandria <lrandria@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/05 13:40:39 by lrandria          #+#    #+#             */
/*   Updated: 2025/05/05 18:46:27 by lrandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_traceroute.h"

static int check_value(const char *str, int flag) {

    char *endptr;
    long val;

    val = strtol(str, &endptr, 10);
    if (flag == OPT_PORT)// PORT is special case, because the limit goes beyond 256
        if (endptr == str || *endptr != '\0' || val > UINT16_MAX || val <= 0)
            oops_crash(E_INVAL_PORT, str);
    else if (endptr == str || *endptr != '\0' || val >= 256 || val <= 0) {
        if (flag == OPT_FIRST_HOP)
            oops_crash(E_IMP_DISTANCE, str);
        else if (flag == OPT_MAX_HOP)
            oops_crash(E_INVAL_HOP, str);
        else if (flag == OPT_NB_PROBES)
            if (val > 10)
                oops_crash(E_INVAL_TRIES, NULL);
        else if (flag == OPT_WAIT)
            oops_crash(E_INVAL_WAIT, str);
    }
    return (uint16_t)val;
}

static void validate_opt(t_parser *options, int flag, char *arg) {
    
    uint16_t val;

    options->flags |= flag; // Setting the flag to '1'
    if (arg) {
        val = check_value(arg, flag);
        if (flag == OPT_FIRST_HOP)
            options->first_hop = val;
        else if (flag == OPT_MAX_HOP)
            options->max_hop = val;
        else if (flag == OPT_PORT)
            options->port = val;
        else if (flag == OPT_WAIT)
            options->wait = val;
    }
}

void parse_args(int argc, char *argv[], t_parser *args) {

    int found_dest = 0;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (strcmp(argv[i], "-?") == 0 || strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "--usage") == 0) {
                print_help();
                exit(EXIT_SUCCESS);
            }
            else if (strcmp(argv[i], "-f") == 0) {
                if (argv[i + 1] == NULL)
                    oops_crash(E_MISSING_ARG, E_TRY_HELP);
                validate_opt(args, OPT_FIRST_HOP, argv[++i]);
            }
            else if (strcmp(argv[i], "-m") == 0) {
                if (argv[i + 1] == NULL)
                    oops_crash(E_MISSING_ARG, E_TRY_HELP);
                validate_opt(args, OPT_MAX_HOP, argv[++i]);
            }
            else if (strcmp(argv[i], "-p") == 0) {
                if (argv[i + 1] == NULL)
                    oops_crash(E_MISSING_ARG, E_TRY_HELP);
                validate_opt(args, OPT_PORT, argv[++i]);
            }
            else if (strcmp(argv[i], "-q") == 0)  {
                if (argv[i + 1] == NULL)
                    oops_crash(E_MISSING_ARG, E_TRY_HELP);
                validate_opt(args, OPT_NB_PROBES, NULL); 
            }
            else if (strcmp(argv[i], "-w") == 0) {
                if (argv[i + 1] == NULL)
                    oops_crash(E_MISSING_ARG, E_TRY_HELP);
                validate_opt(args, OPT_WAIT, argv[++i]);
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