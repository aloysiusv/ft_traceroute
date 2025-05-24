/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   0_main.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lrandria <lrandria@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/05 13:40:31 by lrandria          #+#    #+#             */
/*   Updated: 2025/05/24 22:54:27 by lrandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_traceroute.h"

volatile sig_atomic_t ctrl_c = 0;

static void sig_handler(int sig) {
    (void)sig;
    ctrl_c = 1; // For a cleaner exit
}

static void init_default(t_parser *args) {

    args->first_hop = 1;
    args->max_hop = 30;
    args->port = 33434;
    args->nb_probes = 3;
}

int main(int ac, char *av[]) {

    t_parser        args;
    t_tracert       t;
    
    signal(SIGINT, sig_handler);
    
    if (geteuid() != 0)
        oops_crash(E_NOT_SUDO, NULL);
    if (ac < 2)
        oops_crash(E_MISSING_DEST, E_TRY_HELP);
    memset(&args, 0, sizeof(t_parser));
    memset(&t, 0, sizeof(t_tracert));
    init_default(&args);
    parse_args(ac, av, &args);
    start_traceroute(&args, &t);
    return 0;
}
