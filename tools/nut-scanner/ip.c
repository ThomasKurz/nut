/* ip.c: iterator for IPv4 or IPv6 addresses
 * 
 *  Copyright (C) 2011 - Frederic Bohe <fredericbohe@eaton.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "ip.h"
#include <stdio.h>
#include "common.h"

static void increment_IPv6(struct in6_addr * addr)
{
        addr->s6_addr32[3]=htonl((ntohl(addr->s6_addr32[3])+1));
        if( addr->s6_addr32[3] == 0 ) {
                addr->s6_addr32[2] = htonl((ntohl(addr->s6_addr32[2])+1));
                if( addr->s6_addr32[2] == 0 ) {
                        addr->s6_addr32[1]=htonl((ntohl(addr->s6_addr32[1])+1));
                        if( addr->s6_addr32[1] == 0 ) {
                                addr->s6_addr32[0] =
                                        htonl((ntohl(addr->s6_addr32[0])+1));
                        }
                }
        }
}

static void invert_IPv6(struct in6_addr * addr1, struct in6_addr * addr2)
{
        int i;
        unsigned long addr;

        for( i=0; i<4; i++) {
                addr = addr1->s6_addr32[i];
                addr1->s6_addr32[i] = addr2->s6_addr32[i];
                addr2->s6_addr32[i] = addr;
        }
}

/* Return the first ip or NULL if error */
char * ip_iter_init(ip_iter_t * ip, char * startIP, char * stopIP)
{
	int addr;
	int i;
	char buf[SMALLBUF];

	if( startIP == NULL ) {
		return NULL;
	}

	if(stopIP == NULL ) {
		stopIP = startIP;
	}

	ip->type = IPv4;
	/* Detecting IPv4 vs IPv6 */
	if(!inet_aton(startIP, &ip->start)) {
		/*Try IPv6 detection */
		ip->type = IPv6;
		if(!inet_pton(AF_INET6, startIP, &ip->start6)){
			fprintf(stderr,"Invalid address : %s\n",startIP);
			return NULL;
		}
	}

	/* Compute stop IP */
        if( ip->type == IPv4 ) {
                if(!inet_aton(stopIP, &ip->stop)) {
                        fprintf(stderr,"Invalid address : %s\n",stopIP);
                        return NULL;
                }
        }
        else {
                if(!inet_pton(AF_INET6, stopIP, &ip->stop6)){
                        fprintf(stderr,"Invalid address : %s\n",stopIP);
                        return NULL;
                }
        }

        /* Make sure start IP is lesser than stop IP */
        if( ip->type == IPv4 ) {
                if( ntohl(ip->start.s_addr) > ntohl(ip->stop.s_addr) ) {
                        addr = ip->start.s_addr;
                        ip->start.s_addr = ip->stop.s_addr;
                        ip->stop.s_addr = addr;
                }
		return strdup(inet_ntoa(ip->start));
        }
        else { /* IPv6 */
                for( i=0; i<4; i++ ) {
                        if( ntohl(ip->start6.s6_addr32[i]) !=
                                ntohl(ip->stop6.s6_addr32[i]) ) {
                                if( ntohl(ip->start6.s6_addr32[i]) >
                                        ntohl(ip->stop6.s6_addr32[i])) {
                                        invert_IPv6(&ip->start6,
                                                        &ip->stop6);
                                }
                                break;
                        }
                }
		return strdup(inet_ntop(AF_INET6,&ip->start6,buf,sizeof(buf)));
        }


}

/* return the next IP
return NULL if there is no more IP
*/
char * ip_iter_inc(ip_iter_t * ip)
{
	char buf[SMALLBUF];

	if( ip->type == IPv4 ) {
		/* Check if this is the last address to scan */
		if(ip->start.s_addr == ip->stop.s_addr) {
			return NULL;
		}
		/* increment the address (need to pass address in host
		   byte order, then pass back in network byte order */
		ip->start.s_addr = htonl((ntohl(ip->start.s_addr)+1));

		return strdup(inet_ntoa(ip->start));
	}
	else {
		/* Check if this is the last address to scan */
		if(ip->start6.s6_addr32[0]==ip->stop6.s6_addr32[0]&&
			ip->start6.s6_addr32[1]==ip->stop6.s6_addr32[1]&&
			ip->start6.s6_addr32[2]==ip->stop6.s6_addr32[2]&&
			ip->start6.s6_addr32[3]==ip->stop6.s6_addr32[3]){
			return NULL;
		}

		increment_IPv6(&ip->start6);

		return strdup(inet_ntop(AF_INET6,&ip->start6,buf,sizeof(buf)));
	}
}
