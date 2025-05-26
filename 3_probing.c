/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   3_probing.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lrandria <lrandria@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/07 13:36:18 by lrandria          #+#    #+#             */
/*   Updated: 2025/05/26 13:28:43 by lrandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_traceroute.h"

static int parse_headers(t_tracert *t, uint16_t base_port) {
    struct ip *ip_hdr = (struct ip *)t->response.buffer;
    int ip_header_len = ip_hdr->ip_hl * 4;
    struct icmphdr *icmp_hdr = (struct icmphdr *)(t->response.buffer + ip_header_len);

    // Validate ICMP type
    if (icmp_hdr->type != ICMP_TIME_EXCEEDED && icmp_hdr->type != ICMP_DEST_UNREACH)
        return -1;

    // ICMP payload contains original IP header + UDP header
    struct ip *inner_ip = (struct ip *)(t->response.buffer + ip_header_len + sizeof(struct icmphdr));
    int inner_ip_len = inner_ip->ip_hl * 4;

    // Now get the UDP header inside the embedded packet
    struct udphdr *udp_hdr = (struct udphdr *)((uint8_t *)inner_ip + inner_ip_len);

    uint16_t dport = ntohs(udp_hdr->dest);
    
    // Only accept responses matching our port range
    if (dport < base_port || dport >= base_port + MAX_PROBES)
        return -1;

    return icmp_hdr->type;
}

int probing(t_parser *args, t_tracert *t) {

    int                    finished = 0;
    struct timeval         start, end, timeout;
    struct sockaddr_in     *target = (struct sockaddr_in *)t->resolved->ai_addr;
    fd_set                 readfds;

    for (int probe = 0; probe < args->nb_probes; probe++) {
        target->sin_port = htons(args->port + probe);
        
        gettimeofday(&start, NULL);
        if (sendto(t->sock_udp, t->packet_udp, sizeof(t->packet_udp), 0, t->resolved->ai_addr, t->resolved->ai_addrlen) < 0)
            return -1;
        FD_ZERO(&readfds);
        FD_SET(t->sock_icmp, &readfds);
        timeout.tv_sec = TIMEOUT_SEC;
        timeout.tv_usec = TIMEOUT_USEC;
        int sel = select(t->sock_icmp + 1, &readfds, NULL, NULL, &timeout);
        if (sel < 0)
            return -1;
        else if (sel == 0) {
            fprintf(stdout, "* ");
            continue;
        }
        t_response rsp = t->response;
        memset(&rsp, 0, sizeof(t_response));
        rsp.addr_len = sizeof(rsp.addr);
        if (recvfrom(t->sock_icmp, t->response.buffer, RESPONSE_SIZE, 0, (struct sockaddr *)&rsp.addr, &rsp.addr_len) <= 0)
           fprintf(stdout, "* ");
        gettimeofday(&end, NULL);
        
        int type = parse_headers(t, args->port);
        if (type == -1)
            continue;
        else if (type == ICMP_TIME_EXCEEDED)
            print_probe_result(&start, &end, &rsp.addr, t->response.prev_ip);
        else if (type == ICMP_DEST_UNREACH) {
            print_probe_result(&start, &end, &rsp.addr, t->response.prev_ip);
            finished = 1;      
        }
        if (args->send_wait > 10)
            usleep(args->send_wait);
        else
            sleep(args->send_wait);
    }
    fprintf(stdout, "\n");
    if (finished)
        return END_OF_PROBING;
    return 0;
}