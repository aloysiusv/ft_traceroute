/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_traceroute.h                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lrandria <lrandria@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/14 16:40:23 by lrandria          #+#    #+#             */
/*   Updated: 2025/05/26 19:03:59 by lrandria         ###   ########.fr       */
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
#include <stdbool.h>

// LIBS FOR SOCKETS AND NETWORK
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h> // hints
#include <netinet/udp.h>
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
# define E_BAD_DEST          "ft_traceroute: Temporary failure in name resolution\n Cannot handle \"host\" cmdline arg: "
# define E_MAX_DEST          "ft_traceroute: only one destination needed"
# define E_MISSING_ARG       "ft_traceroute: option requires an argument"
# define E_BAD_ARG           "ft_traceroute: cannot handle option with arg: "
# define E_BAD_FIRSTHOP      "ft_traceroute: first hop out of range"
# define E_BAD_MAXHOP        "ft_traceroute: max hops cannot be more than 255"
# define E_BAD_PROB          "ft_traceroute: no more than 10 probes per hop"
# define E_BAD_PORT          "ft_traceroute: invalid port number"
# define E_BAD_SENDWAIT      "ft_traceroute: bad sendtime specified: "
# define E_WTF_OPT           "ft_traceroute: bad option: "
# define E_SOCKET            "ft_traceroute: 'socket()' error"
# define E_SETSOCKOPT        "ft_traceroute: 'setsockopt()' error"
# define E_BIND              "ft_traceroute: 'bind()' error"
# define E_INTERNAL          "ft_traceroute: internal error"
# define E_TRY_HELP          "\nTry 'ft_traceroute --help' for more information"

// BITWISE FLAGS FOR OPTIONS
# define OPT_FIRST_HOP      (1 << 0)
# define OPT_MAX_HOP        (1 << 1)
# define OPT_PORT           (1 << 2)
# define OPT_NB_PROBES      (1 << 3)
# define OPT_SENDWAIT       (1 << 4)

// FIXED SIZE
# define ICMP_HDR_SIZE      8
# define IP_HDR_SIZE        20
# define PACKET_SIZE        60 // Default for IPV4
# define RESPONSE_SIZE      PACKET_SIZE + IP_HDR_SIZE

// UTILS
# define MAX_PROBES         10
# define END_OF_PROBING     -2
# define TIMEOUT_SEC        1
# define TIMEOUT_USEC       0

typedef struct {
    int                     flags; // For all options
    int                     first_hop; // Set the starting hop of your choice
    int                     max_hop;
    int                     port; // Set dest port (will be incr by 1 for each probe)
    int                     nb_probes;
    int                     send_wait; // Minimal time interval between probes
    char                    *dest;
} t_parser;

typedef struct {
    char                    buffer[RESPONSE_SIZE];
    char                    prev_ip[INET_ADDRSTRLEN];

    struct sockaddr_in      addr;
    socklen_t               addr_len;

    struct icmphdr 		    *icmp_hdr; // To parse ICMP error type
    struct udphdr 		    *udp_hdr; // To parse source port
} t_response;

typedef struct {
    t_response			    response;

    char                    packet_udp[PACKET_SIZE];

    int                     sock_udp; // Sending
    int                     sock_icmp; // Receiving
    char                    *ip_dest;
    struct addrinfo		    *resolved;

} t_tracert;

void	parse_args(int ac, char *av[], t_parser *args);

void	start_traceroute(t_parser *args, t_tracert *t);
int 	probing(t_parser *args, t_tracert *t);

void	print_help();
void    print_probe_result(struct timeval *start, struct timeval *end, struct sockaddr_in *from, char *prev_ip);

void	oops_crash(const char* msg, const char* try_help);

#endif
