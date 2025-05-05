/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   0_main.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lrandria <lrandria@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/05 13:40:31 by lrandria          #+#    #+#             */
/*   Updated: 2025/05/05 16:50:27 by lrandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_traceroute.h"

volatile sig_atomic_t g_stop = 0;

static void init_default(t_parser *args) {
    // Default values
    args->first_hop = 1;
    args->max_hop = 64;
    args->port = 33434;
    args->nb_probes = 3;
    args->wait = 3;
}

void oops_crash(const char* msg, const char* bad_value) {

    fprintf(stderr, "%s", msg);
    if (bad_value)
        fprintf(stderr, "%s", bad_value);
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

static void sig_handler(int sig) {
    (void)sig;
    g_stop = 1; // For a cleaner exit
}

int main(int ac, char *av[]) {

    t_parser       args;
    t_traceroute   tracert;
    
    signal(SIGINT, sig_handler);
    
    if (geteuid() != 0)
        oops_crash(E_NOT_SUDO, NULL);
    if (ac < 2)
        oops_crash(E_MISSING_DEST, E_TRY_HELP);
    memset(&args, 0, sizeof(t_parser));
    memset(&tracert, 0, sizeof(t_traceroute));
    init_default(&args);
    parse_args(ac, av, &args);
    return 0;
    start_traceroute(&args, &tracert);
    return 0;
}
