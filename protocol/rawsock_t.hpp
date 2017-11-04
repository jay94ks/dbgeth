/*
 * rawsock_t.hpp
 *
 *  Created on: 2017. 11. 3.
 *      Author: jaehoon
 *      Blog  : http://www.jayks.ml/
 *      Email : jay94ks@gmail.com
 *
	The MIT License
	Copyright (c) 2017 jay94ks@gmail.com

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:
	The above copyright notice and this permission notice shall be included in
	all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	THE SOFTWARE.
 */

#ifndef RAWSOCK_T_HPP_
#define RAWSOCK_T_HPP_

#include <stdint.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <ifaddrs.h>

class rawsock_t {
public:
	rawsock_t()
	: m_socket(::socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))),
	  m_frameSize(ETH_FRAME_LEN), m_sendBuffer(0), m_recvBuffer(0)
	{
		memset(m_nic, 0, sizeof(m_nic));
		memset(&m_saPreset, 0, sizeof(m_saPreset));
		memset(&m_ethPreset, 0, sizeof(m_ethPreset));

		m_saPreset.sll_halen = ETH_ALEN;
		m_saPreset.sll_family = AF_PACKET;
		m_saPreset.sll_hatype = ARPHRD_ETHER;
		m_saPreset.sll_pkttype = PACKET_OTHERHOST;
		m_saPreset.sll_protocol = htons(ETH_P_ALL);
		m_saPreset.sll_ifindex = -1;
	}

	~rawsock_t() {
		if(m_socket >= 0)
			::close(m_socket);

		if(m_sendBuffer)
			delete[] (m_sendBuffer);

		if(m_recvBuffer)
			delete[] (m_recvBuffer);

		m_socket = -1;
		m_sendBuffer = m_recvBuffer = 0;
	}

public:
	static uint8_t* mac_broadcast() {
		return (uint8_t*) "\xff\xff\xff\xff\xff\xff";
	}

	static uint8_t* mac_null() {
		return (uint8_t*) "\x00\x00\x00\x00\x00\x00";
	}

private:
	int32_t		m_socket;
	sockaddr_ll	m_saPreset;
	ethhdr			m_ethPreset;
	char			m_nic[64];
	int32_t		m_frameSize;
	uint8_t*		m_sendBuffer;
	uint8_t*		m_recvBuffer;
	uint8_t		m_lastestSource[ETH_ALEN];

private:
	uint8_t* rbuffer() {
		if(m_recvBuffer)
			return m_recvBuffer;

		return m_recvBuffer = new uint8_t[m_frameSize];
	}

	uint8_t* sbuffer() {
		if(m_sendBuffer)
			return m_sendBuffer;

		return m_sendBuffer = new uint8_t[m_frameSize];
	}

public:
	rawsock_t& frame(int32_t newSize) {
		if(newSize == m_frameSize)
			return *this;

		if(m_sendBuffer)
			delete[] (m_sendBuffer);

		if(m_recvBuffer)
			delete[] (m_recvBuffer);

		m_sendBuffer = 0;
		m_recvBuffer = 0;
		return *this;
	}

	int32_t frame() { return m_frameSize; }
	int32_t payload() { return m_frameSize - sizeof(ethhdr); }
	int32_t fd() { return m_socket; }

public:
	bool nic(const char* nic) {
		ifaddrs* ifa,* ifp;

		if(nic == 0) {
			memset(m_nic, 0, sizeof(m_nic));
			memset(m_saPreset.sll_addr, 0, ETH_ALEN);
			return true;
		}

		strcpy(m_nic, nic);
		getifaddrs(&ifa);
		if(!ifa) return false;

		for(ifp = ifa; ifp != 0; ifp = ifp->ifa_next) {
			if(ifp->ifa_addr->sa_family == AF_PACKET &&
				strcmp(ifp->ifa_name, nic) == 0)
			{
				sockaddr_ll* p = (sockaddr_ll*) ifp->ifa_addr;
				m_saPreset.sll_ifindex = p->sll_ifindex;
				this->local(p->sll_addr);
				freeifaddrs(ifa); ifa = 0;
				break;
			}
		}

		if(ifa) {
			freeifaddrs(ifa);
			return false;
		}
		return true;
	}

	rawsock_t& remote(uint8_t* macAddr) {
		memcpy(m_ethPreset.h_dest, macAddr, ETH_ALEN);
		memcpy(m_saPreset.sll_addr, macAddr, ETH_ALEN);
		return *this;
	}

	rawsock_t& local(uint8_t* macAddr) {
		memcpy(m_ethPreset.h_source, macAddr, ETH_ALEN);
		return *this;
	}

	rawsock_t& proto(uint16_t p) {
		m_ethPreset.h_proto = htons(p);
		m_saPreset.sll_protocol = htons(p);
		return *this;
	}

	const char* nic() { return m_nic; }
	uint8_t* remote() { return m_ethPreset.h_dest; }
	uint8_t* local() { return m_ethPreset.h_source; }
	uint16_t proto() { return ntohs(m_ethPreset.h_proto); }
	uint8_t* sender() { return m_lastestSource; }

public:
	int32_t send(void* payload, uint16_t length) {
		uint8_t* frame = sbuffer();
		sockaddr_ll sa = m_saPreset;
		int32_t retVal = 0;

		// build the frame...
		length = length > m_frameSize - sizeof(ethhdr) ?
				m_frameSize - sizeof(ethhdr) : length;

		memcpy(frame, &m_ethPreset, sizeof(m_ethPreset));
		memcpy(frame + sizeof(m_ethPreset), payload, length);

		if(strlen(m_nic)) {
			retVal = sendto(m_socket, frame, m_frameSize,
					0, (sockaddr*) &sa, sizeof(m_saPreset));
		}

		else {
			ifaddrs* ifa,* ifp;
			getifaddrs(&ifa);

			for(ifp = ifa; ifp != 0; ifp = ifp->ifa_next) {
				if(ifp->ifa_addr->sa_family == AF_PACKET) {
					sockaddr_ll* p = (sockaddr_ll*) ifp->ifa_addr;
					ethhdr* ep = (ethhdr*) frame;

					sa.sll_ifindex = p->sll_ifindex;
					memcpy(ep->h_source, p->sll_addr, ETH_ALEN);

					retVal = sendto(m_socket, frame, m_frameSize,
							0, (sockaddr*) &sa, sizeof(m_saPreset));
				}
			}

			freeifaddrs(ifa);
		}

		return retVal;
	}

	int32_t recv(void* payload, uint16_t length, int32_t timeout = -1) {
		uint8_t* frame = rbuffer();
		socklen_t salen = sizeof(sockaddr_ll);
		sockaddr_ll sa = { 0, };
		int32_t retVal = -1;
		ethhdr* eth = (ethhdr*) frame;
		time_t begin = 0;

		length = length > m_frameSize - sizeof(ethhdr) ?
				m_frameSize - sizeof(ethhdr) : length;

		time(&begin);
		while(true) {
			retVal = recvfrom(m_socket, frame,
					m_frameSize, 0, (sockaddr*) &sa, &salen);

			if(retVal < 0)
				break;

			if(memcmp(remote(), mac_null(), ETH_ALEN) == 0) {
				memcpy(m_lastestSource, eth->h_source, ETH_ALEN);
				memcpy(payload, eth + 1, length);
				break;
			}

			if(memcmp(eth->h_dest, local(), ETH_ALEN) == 0 ||
				(memcmp(eth->h_source, local(), ETH_ALEN) != 0 &&
				 memcmp(eth->h_dest, mac_broadcast(), ETH_ALEN) == 0))
			{
				memcpy(m_lastestSource, eth->h_source, ETH_ALEN);
				memcpy(payload, eth + 1, length);

				break;
			}

			if(timeout > 0) {
				time_t cur = 0;
				time(&cur);

				if(timeout < cur - begin) {
					retVal = -2;
					break;
				}
			}
		}

		return retVal;
	}
};

#endif /* RAWSOCK_T_HPP_ */
