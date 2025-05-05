/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_traceroute.h                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lrandria <lrandria@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/14 16:40:23 by lrandria          #+#    #+#             */
/*   Updated: 2025/05/05 17:05:56 by lrandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FT_TRACEROUTE_H
#define FT_TRACEROUTE_H

// STANDARD LIBS FOR PARSING AND PRINTING
#include <unistd.h> 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

// LIBS FOR SOCKETS AND NETWORK
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h> // hints
#include <sys/time.h> // timeval
#include <netdb.h>
#include <sys/select.h>

// FOR CTRL-C
#include <signal.h>

// FOR CALCULATING STATS
#include <math.h>

// ERRORS MSG
# define E_NOT_SUDO          "ft_traceroute: are you sudo...?"
# define E_MISSING_DEST      "ft_traceroute: missing host operand"
# define E_BAD_DEST          "ft_traceroute: unknown host"
# define E_MAX_DEST          "ft_traceroute: only one destination needed"
# define E_MISSING_ARG       "ft_traceroute: option requires an argument -- "
# define E_IMP_DISTANCE      "ft_traceroute: impossible distance: "
# define E_INVAL_HOP         "ft_traceroute: invalid hops value: "
# define E_INVAL_PORT        "ft_traceroute: invalid port number: "
# define E_INVAL_TRIES       "ft_traceroute: number of tries should be between 1 and 10"
# define E_INVAL_WAIT        "ft_traceroute: ridiculous waiting time: " // max is 60 sec
# define E_WTF_OPT           "ft_traceroute: unrecognised option: "
# define E_SOCK_ERROR        "ft_traceroute: 'socket()' error"
# define E_SETSOCKOPT_ERROR  "ft_traceroute: 'setsockopt()' error"
# define E_INTERNAL_ERROR    "ft_traceroute: internal error"
# define E_TRY_HELP          "\nTry 'ft_traceroute -?', 'ft_traceroute --help' or 'ft_traceroute --usage' for more information"

// BITWISE FLAGS FOR OPTIONS
# define OPT_FIRST_HOP  0b1
# define OPT_MAX_HOP    0b10
# define OPT_PORT       0b100
# define OPT_NB_PROBES  0b1000
# define OPT_WAIT       0b10000

// FIXED SIZE
# define ICMP_HDR_SIZE  8
# define IP_HDR_SIZE    20
# define PAYLOAD_SIZE   56
# define PACKET_SIZE    PAYLOAD_SIZE + ICMP_HDR_SIZE
# define RESPONSE_SIZE  PACKET_SIZE + IP_HDR_SIZE

typedef struct {
    int                 flags; // For all options
    int                 first_hop; // Set the starting hop of your choice
    int                 max_hop;
    int                 port;
    int                 nb_probes;
    int                 wait; // How long you wait for a response
    char                *dest;
} t_parser;

typedef struct {
    char                buffer[RESPONSE_SIZE];
    struct iphdr 		*ip_hdr;
    int                 ip_hdr_len;
    struct icmphdr 		*icmp_hdr;
} t_response;

typedef struct {
    t_response			response;
    
    int                 sock_send_udp;
    int                 sock_rcv_icmp;
    char                *ip_dest;
    struct addrinfo		*resolved;
    
    // Stats to print
    int                 packets_lost;
    int                 packets_sent;
    double              rtt_min;
    double              rtt_max;
    double              rtt_sum;
    double              rtt_sum_sqr;
} t_traceroute;

void	parse_args(int ac, char *av[], t_parser *args);

void	start_traceroute(t_parser *args, t_traceroute *tracert);
int 	play_tracert_pong(t_parser *args, t_traceroute *tracert);

void	print_start_infos(const t_parser *args, const t_traceroute *tracert);
void	print_response_infos(const t_response *rsp, const int bytes, double rtt);
void 	print_end_infos(const t_traceroute *tracert, char *dest);
void	print_errors(t_traceroute *tracert, const int bytes, const int flags);
void	print_help();

void	oops_crash(const char* msg, const char* try_help);

#endif
