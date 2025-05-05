/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   3_tracert_pong.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lrandria <lrandria@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/03 10:50:35 by lrandria          #+#    #+#             */
/*   Updated: 2025/05/05 13:39:51 by lrandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_traceroute.h"

static void parse_headers(t_response *rsp) {
        
    // Parse IP and ICMP header
    rsp->ip_hdr = (struct iphdr *)rsp->buffer;
    rsp->ip_hdr_len = rsp->ip_hdr->ihl * 4;
    rsp->icmp_hdr = (struct icmphdr *)(rsp->buffer + rsp->ip_hdr_len);
}

static int receive_pong(int sockfd, t_response *rsp) {
    
    int                 ret;
    struct sockaddr_in  recv_addr;
    socklen_t           recv_len = sizeof(recv_addr);
    
    memset(rsp, 0, sizeof(t_response));
    ret = recvfrom(sockfd, rsp->buffer, RESPONSE_SIZE, 0, (struct sockaddr *)&recv_addr, &recv_len);
    return ret; // Return nb of bytes, or < 0 is error
}

static int send_tracert(t_traceroute *tracert, struct addrinfo *resolved, int linger) {

    if (sendto(traceroute->sockfd, &traceroute->packet, sizeof(traceroute->packet), 0, resolved->ai_addr, resolved->ai_addrlen) < 0)
        return -1;
   
    // Init fds
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(traceroute->sockfd, &readfds);
    
    struct timeval timeout;
    timeout.tv_sec = linger; // Arg is passed as seconds
    timeout.tv_usec = 0;
    int ret = select(traceroute->sockfd + 1, &readfds, NULL, NULL, &timeout);
    return ret; // -2 = sigint, -1 = error, 0 = timeout, 1 = ready to read
}

int play_tracert_pong(t_parser *args, t_traceroute *tracert) {
    
    int             ret_send, ret_rcv;
    struct timeval  start, end;
    
    // Start clock and send ICMP request
    gettimeofday(&start, NULL);
    ret_send = send_tracert(tracert, traceroute->resolved, args->linger);
    if (ret_send <= 0)
        return -1;
    ret_rcv = receive_pong(traceroute->sockfd, &traceroute->response);
    if (ret_rcv <= 0)
        return -1;
    gettimeofday(&end, NULL);

    // Check if we have the "success" header
    parse_headers(&traceroute->response);
    if (traceroute->response.icmp_hdr->type == ICMP_ECHOREPLY) {
	    double rtt = (end.tv_sec - start.tv_sec) * 1000.0 + (double)(end.tv_usec - start.tv_usec) / 1000.0;

	    if (!(args->flags & OPT_QUIET))
		    print_response_infos(&traceroute->response, ret_rcv, rtt);

	    // Update RTT stats
	    if (rtt < traceroute->rtt_min || traceroute->rtt_min == 0.0)
		    traceroute->rtt_min = rtt;
	    if (rtt > traceroute->rtt_max)
		    traceroute->rtt_max = rtt;
	    traceroute->rtt_sum += rtt;
	    traceroute->rtt_sum_sqr += rtt * rtt;
    } 
    else {
	    print_errors(tracert, ret_rcv, args->flags);
	    return -1;
    }
    return 0;
}