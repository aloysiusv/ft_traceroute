/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   2_init_start.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lrandria <lrandria@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/05 13:40:46 by lrandria          #+#    #+#             */
/*   Updated: 2025/05/26 20:14:49 by lrandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_traceroute.h"

extern volatile sig_atomic_t ctrl_c;

static char *get_ip_dest(t_tracert *t, struct addrinfo *resolved) {
    
    static char         ip_addr[INET_ADDRSTRLEN];
    struct sockaddr_in  *tmp = (struct sockaddr_in *)resolved->ai_addr;
    
    if (inet_ntop(AF_INET, &tmp->sin_addr, ip_addr, sizeof(ip_addr)) == NULL) {
        close(t->sock_udp);
        close(t->sock_icmp);
        freeaddrinfo(resolved);
        oops_crash(E_BAD_DEST, NULL);
    }   
    return ip_addr;
}

static struct addrinfo *resolve_dest(t_tracert *t, char *dest) {
    
    struct addrinfo hints;
    struct addrinfo *resolved;
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_RAW;
    hints.ai_protocol = IPPROTO_ICMP;

    if (getaddrinfo(dest, NULL, &hints, &resolved) != 0) {
        close(t->sock_udp);
        close(t->sock_icmp);
        oops_crash(E_BAD_DEST, dest);
    }
    return resolved;
}

// static void init_src_port(int *sock_udp) {

//     struct sockaddr_in src_addr = {0};
//     src_addr.sin_family = AF_INET;
//     src_addr.sin_addr.s_addr = INADDR_ANY;
//     src_addr.sin_port = htons((getpid() & 0xFFFF) | (1 << 15));
    
//     bind(*sock_udp, (struct sockaddr *)&src_addr, sizeof(src_addr));
// }

static void init_sockets(int *sock_udp, int *sock_icmp) {
    
    *sock_udp = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    *sock_icmp = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

    if (*sock_udp < 0)
        oops_crash(E_SOCKET, NULL);
    if (*sock_icmp < 0) {
        close(*sock_udp);
        oops_crash(E_SOCKET, NULL);
    }
}

void start_traceroute(t_parser *args, t_tracert *t) {
    
    // Preparing socket and dest address
    init_sockets(&t->sock_udp, &t->sock_icmp);
    //init_src_port(&t->sock_udp);
    t->resolved = resolve_dest(t, args->dest);
    t->ip_dest = get_ip_dest(t, t->resolved);\

    // Main loop
    fprintf(stdout, "traceroute to %s (%s), %d hops max, %d byte packets\n", args->dest, t->ip_dest, args->max_hop, PACKET_SIZE);
    for (; args->first_hop < args->max_hop + 1; args->first_hop++) { // +1 to print last hop
        fprintf(stdout, "%2d  ", args->first_hop);
        if (setsockopt(t->sock_udp, IPPROTO_IP, IP_TTL, &args->first_hop, sizeof(args->first_hop)) < 0) {
            fprintf(stderr, E_SETSOCKOPT);
            break;
        }
        int ret = probing(args, t);
        if (ret < 0 || ret == END_OF_PROBING)
            break;
        if (ctrl_c == 1)
            break;
    }
    freeaddrinfo(t->resolved);
    close(t->sock_udp);
    close(t->sock_icmp);
}
