/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   3_probing.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lrandria <lrandria@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/07 13:36:18 by lrandria          #+#    #+#             */
/*   Updated: 2025/05/29 11:56:30 by lrandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_traceroute.h"

static bool is_valid_response(t_response *rsp, uint16_t expected_sport, uint16_t expected_dport) {
    
    if (!rsp->icmp_hdr || !rsp->udp_hdr)
        return false;
    if (rsp->icmp_hdr->type != ICMP_TIME_EXCEEDED && rsp->icmp_hdr->type != ICMP_DEST_UNREACH)
        return false;
    if (ntohs(rsp->udp_hdr->uh_sport) != expected_sport)
        return false;
    if (ntohs(rsp->udp_hdr->uh_dport) != expected_dport)
        return false;

    return true;
}

static void parse_headers(t_response *rsp, char *buffer) {

    // Get ICMP header
    struct ip *ip_hdr = (struct ip *)buffer;
    int ip_header_len = ip_hdr->ip_hl * 4;
    rsp->icmp_hdr = (struct icmphdr *)(rsp->buffer + ip_header_len);

    // Get UDP header
    struct ip *inner_ip = (struct ip *)(rsp->buffer + ip_header_len + sizeof(struct icmphdr));
    int inner_ip_len = inner_ip->ip_hl * 4;
    rsp->udp_hdr = (struct udphdr *)((uint8_t *)inner_ip + inner_ip_len);
}

static int receive_probe(t_response *rsp, int *sock_icmp, uint16_t expected_sport, uint16_t expected_dport) {

    fd_set          readfds;
    struct timeval  timeout = {.tv_sec = TIMEOUT_SEC, .tv_usec = TIMEOUT_USEC};
    
    while (1) { // Ugly, I know :'(
        FD_ZERO(&readfds);
        FD_SET(*sock_icmp, &readfds);
        int ret = select(*sock_icmp + 1, &readfds, NULL, NULL, &timeout);
        if (ret <= 0)
            return ret; // Error or timeout

        rsp->addr_len = sizeof(rsp->addr);
        ssize_t bytes = recvfrom(*sock_icmp, rsp->buffer, RESPONSE_SIZE, 0, (struct sockaddr *)&rsp->addr, &rsp->addr_len);
        if (bytes < 0)
            continue;

        parse_headers(rsp, rsp->buffer);
        if (is_valid_response(rsp, expected_sport, expected_dport))
            return 1;
    }
    return 0;
}

int probing(t_parser *args, t_tracert *t) {

    t_response          rsp = t->response;
    int                 finished = 0;
    struct timeval      start, end;

    for (int probe = 0; probe < args->nb_probes; probe++) {
        uint16_t current_port = args->port + probe;

        ((struct sockaddr_in *)t->resolved->ai_addr)->sin_port = htons(current_port);
        memset(t->packet_udp, 0, sizeof(t->packet_udp));

        // Start chrono
        gettimeofday(&start, NULL);
        if (sendto(t->sock_udp, t->packet_udp, sizeof(t->packet_udp), 0, t->resolved->ai_addr, t->resolved->ai_addrlen) < 0)
            return -1;
        int ret_rcv = receive_probe(&rsp, &t->sock_icmp, t->sport, current_port);
        if (ret_rcv < 0)
            return -1;
        else if (ret_rcv == 0) {
            fprintf(stdout, "* ");
            continue;
        }
        gettimeofday(&end, NULL);
        
        print_probe_result(&start, &end, &rsp.addr, rsp.prev_ip);
        fflush(stdout); // Display sequentially
        
        if (rsp.icmp_hdr->type == ICMP_DEST_UNREACH)
            finished = 1;

        // Optional interval between probes
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
