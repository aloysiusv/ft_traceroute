/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   2_init_start.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lrandria <lrandria@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/05 13:40:46 by lrandria          #+#    #+#             */
/*   Updated: 2025/05/05 18:19:57 by lrandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_traceroute.h"

extern volatile sig_atomic_t g_stop;

uint16_t checksum(void *ptr, int len)
{
    uint16_t* data = ptr;
    uint32_t  sum = 0;
    for (; len > 1; len -= 2)
        sum += *data++;
    if (len == 1)
        sum += *(uint8_t*)data;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    return ~sum;
}

static t_packet init_packet(int i) {
    
    t_packet packet;
    
    memset(&packet, 0, sizeof(packet));
    packet.type = ICMP_ECHO;
    packet.code = 0;
    packet.id = ntohs(getpid() & 0xFFFF); // Making sure it fits in 16-bits range
    packet.seq = i;
    packet.checksum = checksum(&packet, sizeof(packet));
    return packet;
}

static char *get_ip(struct addrinfo *resolved, int sock) {
    
    static char         ip_addr[INET_ADDRSTRLEN];
    struct sockaddr_in  *tmp = (struct sockaddr_in *)resolved->ai_addr;
    
    if (inet_ntop(AF_INET, &tmp->sin_addr, ip_addr, sizeof(ip_addr)) == NULL) {
        close(sock);
        freeaddrinfo(resolved);
        oops_crash(E_BAD_DEST, NULL);
    }   
    return ip_addr;
}

static struct addrinfo *resolve_addr(char *dest, int sock) {
    
    struct addrinfo hints;
    struct addrinfo *resolved;
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_RAW;
    hints.ai_protocol = IPPROTO_ICMP;

    if (getaddrinfo(dest, NULL, &hints, &resolved) != 0) {
        close(sock);
        oops_crash(E_BAD_DEST, NULL);
    }
    return resolved;
}

static int init_socket(t_parser *args) {
    
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    
    if (sockfd < 0)
        oops_crash(E_SOCK_ERROR, NULL);
    if (setsockopt(sockfd, IPPROTO_IP, IP_TTL, &args->ttl, sizeof(args->ttl)) < 0) {
        close(sockfd);
        oops_crash(E_SETSOCKOPT_ERROR, NULL);
    }
    return sockfd;
}

void start_traceroute(t_parser *args, t_traceroute *tracert) {
    
    // Preparing socket and dest address
    traceroute->sockfd = init_socket(args);
    traceroute->resolved = resolve_addr(args->dest, traceroute->sockfd);
    traceroute->ip_dest = get_ip(traceroute->resolved, traceroute->sockfd);
    print_start_infos(args, tracert);
    
    // Main loop
    for (int i = 0; i != args->packet_count; i++) {
        traceroute->packet = init_packet(i);
        if (play_tracert_pong(args, tracert) == -1)
            traceroute->packets_lost++;
        else
            traceroute->packets_sent++;
        sleep(args->interval);
        if (g_stop == 1)
            break;
    }
    print_end_infos(tracert, args->dest);
    freeaddrinfo(traceroute->resolved);
    close(traceroute->sockfd);
}
