/*
 * main.cpp
 *
 *  Created on: 2017. 11. 4.
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
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/utsname.h>
#include "protocol/rawsock_t.hpp"
#include "protocol/dbgeth_pkt.hpp"
#include "protocol/packetbuilder.hpp"
#include "protocol/packetreader.hpp"
#include <sys/signal.h>

int signaller_main(const char* nic, const char* script);
int listener_main(const char* nic, const char* script);
int main(int argc, char** argv) {
	bool listener = false;
	const char* nic = 0;
	const char* script = 0;

	if(argc < 2) {
		// usage
		fprintf(stderr,	"dbgeth - debugging helper through ethernet - v0.01 alpha\n"
							"developer blog: http://www.jayks.ml / email: jay94ks@gmail.com\n"
							"copyright (c) 2017 dbgeth, developed by jay94ks@gmail.com.\n"
							"this software is under the MIT License, refer LICENSE file in source code.\n"
							"\n"
							"usage: dbgeth NIC -s [SCRIPT] : signaller mode.\n"
							"     : dbgeth NIC -l [SCRIPT] : listener mode.\n");
		return 0;
	}

	else if(argc >= 3 &&
		(strcmp(argv[2], "-s") == 0 ||
		(listener = (strcmp(argv[2], "-l") == 0))))
	{
		if(getuid() != 0 && geteuid() != 0) {
			fprintf(stderr, "error: permission denied.\n");
			return -1;
		}

		nic = argv[1];
		script = argc > 3 ? argv[3] : 0;
	}

	else {
		//error.
		fprintf(stderr, "error: invalid argument or invalid usage.\n");
		return -1;
	}

	if(listener)
		listener_main(nic, script);

	else signaller_main(nic, script);

	return 0;
}

int listener_main(const char* nic, const char* script) {
	packetbuilder pktbuild;
	utsname uts;
	uname(&uts);

	rawsock_t rawsock;
	rawsock.remote(rawsock_t::mac_broadcast())
			.proto(rawsock.frame())
			.nic(nic);

	char* payload = new char[rawsock.payload()];

	pktbuild.add_query(0, E_DBGQRY_DISCOVERY,
			E_DBGFLG_RESPONSE | E_DBGFLG_FINAL, &uts, sizeof(uts));

	pktbuild.calc_chkavg();

	while(true) {
		rawsock.recv(payload, rawsock.payload());
		packetreader rd (payload, rawsock.payload());
		if(rd.header()->magic == DBGETH_MAGIC) {
			while(rd.query()) {
				dbgeth_pkt_query* query = rd.query();
				fflush(stdout);

				if(query->type == E_DBGQRY_DISCOVERY &&
					(query->flags & E_DBGFLG_REQUEST))
				{
					struct utsname* src = (utsname*)(query + 1);
					printf("dbgeth: signal arrived.\n"
							"attached system information:\n");
					printf(" * operating system: %s\n", src->sysname);
					printf(" * hardware type   : %s\n", src->machine);
					printf(" * node name       : %s\n", src->nodename);
					printf(" * implementation  : %s\n", src->release);
					printf(" * version         : %s\n", src->version);
					printf(" * mac address     : %02x %02x %02x %02x %02x %02x\n",
							rawsock.sender()[0], rawsock.sender()[1],
							rawsock.sender()[2], rawsock.sender()[3],
							rawsock.sender()[4], rawsock.sender()[5]);

					rawsock.send(pktbuild.buffer(), pktbuild.length());

					printf("executing the script(%s)...\n", script == 0 ? "nothing" : script);
					if(script) system(script);
					return 0;
				}

				rd.next();
			}
		}
	}
}

int signaller_main(const char* nic, const char* script) {
	packetbuilder pktbuild;
	utsname uts;
	uname(&uts);

	rawsock_t rawsock;
	rawsock.remote(rawsock_t::mac_broadcast())
			.proto(rawsock.frame())
			.nic(nic);

	pid_t pid = fork();

	if(pid == 0) {
		pktbuild.add_query(0, E_DBGQRY_DISCOVERY,
				E_DBGFLG_REQUEST | E_DBGFLG_REPEAT, &uts, sizeof(uts));

		pktbuild.calc_chkavg();

		while(true) {
			rawsock.send(pktbuild.buffer(), pktbuild.length());
			sleep(1);
		}
	}

	else {
		char* buffer = new char[rawsock.payload()];
		while(true) {
			rawsock.recv(buffer, rawsock.payload());
			packetreader rd (buffer, rawsock.payload());
			if(rd.header()->magic == DBGETH_MAGIC) {

				while(rd.query()) {
					dbgeth_pkt_query* query = rd.query();

					if(query->type == E_DBGQRY_DISCOVERY &&
						(query->flags & E_DBGFLG_RESPONSE))
					{
						struct utsname* src = (utsname*)(query + 1);
						printf("dbgeth: signal arrived.\n"
								"attached system information:\n");

						printf(" * operating system: %s\n", src->sysname);
						printf(" * hardware type   : %s\n", src->machine);
						printf(" * node name       : %s\n", src->nodename);
						printf(" * implementation  : %s\n", src->release);
						printf(" * version         : %s\n", src->version);
						printf(" * mac address     : %02x %02x %02x %02x %02x %02x\n",
								rawsock.sender()[0], rawsock.sender()[1],
								rawsock.sender()[2], rawsock.sender()[3],
								rawsock.sender()[4], rawsock.sender()[5]);

						printf("okay, the programmed script will be executed as soon as possible.\n");

						kill(pid, SIGTERM);
						fflush(stdout);

						if(script) system(script);
						return 0;
					}

					rd.next();
				}
			}

		}
	}
}

