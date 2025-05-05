/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   4_print.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lrandria <lrandria@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/03 10:50:43 by lrandria          #+#    #+#             */
/*   Updated: 2025/05/05 13:40:21 by lrandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_traceroute.h"

void print_help() {
   
	fprintf(stdout, "Usage: ft_traceroute [OPTION...] HOST ...\n"
					"Print the route packets trace to network host.\n\n"
					"Options:\n\n"
					"   -c,                        (count) stop after sending X packets\n"
					"   -i,                        (interval) wait X seconds between sending each packet\n"
					"   --ttl=N                    specify N as time-to-live\n"
					"   -W,                        (linger) number of seconds to wait for response\n"
					"   -v,                        verbose output\n"
					"   -q,                        quiet output\n"
					"   -?, --help OR --usage      display this help list\n");
}

void print_start_infos(const  t_parser *args, const t_traceroute *tracert) {

	if (args->flags & OPT_VERBOSE) {
		int packet_id = getpid() & 0xFFFF;
		printf("tracert %s (%s): %d data bytes, id 0x%x = %d\n", args->dest, traceroute->ip_dest, PAYLOAD_SIZE, packet_id, packet_id);
	}
	else
		printf("tracert %s (%s): %d data bytes\n", args->dest, traceroute->ip_dest, PAYLOAD_SIZE);
}

void print_response_infos(const t_response *rsp, const int bytes, const double rtt) {

	char ip_addr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &rsp->ip_hdr->saddr, ip_addr, sizeof(ip_addr));

    printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n",
        bytes - rsp->ip_hdr_len,
        ip_addr,
        rsp->icmp_hdr->un.echo.sequence,
        rsp->ip_hdr->ttl,
        rtt
    );
}

void print_errors(t_traceroute *tracert, const int bytes,const int flags) {

	t_response *rsp = &traceroute->response;
	
	if (rsp->icmp_hdr->type == ICMP_TIME_EXCEEDED)
		printf("%d bytes from %s: Time to live exceeded\n", bytes - rsp->ip_hdr_len, inet_ntoa((struct in_addr){rsp->ip_hdr->saddr}));
	else if (rsp->icmp_hdr->type == ICMP_DEST_UNREACH)
		printf("%d bytes from %s: Destination Host Unreachable\n", bytes - rsp->ip_hdr_len, inet_ntoa((struct in_addr){rsp->ip_hdr->saddr}));
	
	if (flags & OPT_VERBOSE) {
		// Parse the second IP header (you get two when getting an error)
		struct iphdr *inner_ip = (struct iphdr *)(rsp->icmp_hdr + 1);

		// Check response size, in case it's a bad header
		if ((uint8_t *)inner_ip - (uint8_t *)rsp->buffer + sizeof(struct iphdr) <= RESPONSE_SIZE) {
			const uint8_t  *buf = (const uint8_t *)inner_ip;
			int            inner_ip_len = inner_ip->ihl * 4;
			
			// Get src and dst properly
			char src[INET_ADDRSTRLEN];
			char dst[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &inner_ip->saddr, src, sizeof(src));
			inet_ntop(AF_INET, &inner_ip->daddr, dst, sizeof(dst));

			printf("IP Hdr Dump:\n");
			for (int i = 0; i < inner_ip_len; i += 2)
				printf(" %02x%02x ", buf[i], buf[i + 1]);
			printf("\n");

			printf("Vr HL TOS  Len   ID Flg  off TTL Pro  cks      Src	Dst	Data\n");
			printf(" %x  %x  %02x %04x %04x   %x %04x  %02x  %02d %04x %s  %s\n",
				inner_ip->version,
				inner_ip->ihl,
				inner_ip->tos,
				ntohs(inner_ip->tot_len),
				ntohs(inner_ip->id),
				ntohs(inner_ip->frag_off) >> 13,
				(ntohs(inner_ip->frag_off) & 0x1FFF),
				inner_ip->ttl,
				inner_ip->protocol,
				ntohs(inner_ip->check),
				src,
				dst
			);
			printf("ICMP: type %d, code %d, size %d, id 0x%04x, seq 0x%04x\n",
				traceroute->packet.type,
				traceroute->packet.code,
				PACKET_SIZE,
				traceroute->packet.id,
				traceroute->packet.seq
			);
		}
	}
}

void print_end_infos(const t_traceroute *tracert, char *dest) {
	
	int     total_packets = traceroute->packets_sent + traceroute->packets_lost;
	double  mean = traceroute->rtt_sum / total_packets;
	double  stddev = sqrt((traceroute->rtt_sum_sqr / total_packets) - pow(mean, 2));
	int     loss = traceroute->packets_lost / total_packets * 100;

	printf("--- %s ft_traceroute statistics ---\n", dest);
	printf("%d packets transmitted, %d packets received, %d%% packet loss\n", total_packets, traceroute->packets_sent, loss);
	if (loss != 100)
		printf("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n",traceroute->rtt_min, mean, traceroute->rtt_max, stddev);
}