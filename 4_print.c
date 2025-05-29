/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   4_print.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lrandria <lrandria@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/03 10:50:43 by lrandria          #+#    #+#             */
/*   Updated: 2025/05/29 12:13:01 by lrandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_traceroute.h"

void oops_crash(const char* msg, const char* bad_value) {

    fprintf(stderr, "%s", msg);
    if (bad_value)
        fprintf(stderr, "%s", bad_value);
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

void print_help() {
   
    fprintf(stdout, "Usage: ft_traceroute [OPTION...] HOST ...\n"
                    "Print the route packets trace to network host.\n\n"
                    "Options:\n\n"
                    "   -f,                        (first ttl) Specifies with what TTL to start. Defaults to 1.\n"
                    "   -m,                        (max ttl/max hops) Specifies the maximum number of hops (max time-to-live value) traceroute will probe. The default is 30.\n"
                    "   -p                         (port) Specifies the destination port base traceroute will use (the destination port number will be incremented by each probe).\n"
                    "   -q,                        (queries) Sets the number of probe packets per hop. The default is 3.\n"
                    "   -z,                        Minimal time interval between probes (default 0) in seconds. If the value is more than 10, then it specifies a number in milliseconds.\n"
                    "   --help                     display this help list\n");
}

void print_probe_result(struct timeval *start, struct timeval *end, struct sockaddr_in *from, char *prev_ip) {
    
    char    current_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(from->sin_addr), current_ip, sizeof(current_ip));

    if (strcmp(prev_ip, current_ip) != 0) {
        fprintf(stdout, "%s  ", current_ip);
        strcpy(prev_ip, current_ip); // Avoid printing the same IP
    }

    double  rtt;
    rtt = (end->tv_sec - start->tv_sec) * 1000.0;
    rtt += (end->tv_usec - start->tv_usec) / 1000.0;
    fprintf(stdout, "%.3f ms  ",rtt);
}