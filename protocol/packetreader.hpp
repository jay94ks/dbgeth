/*
 * packetreader.hpp
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
 */

#ifndef PROTOCOL_PACKETREADER_HPP_
#define PROTOCOL_PACKETREADER_HPP_

#include <stdint.h>
#include <stdlib.h>
#include "rawsock_t.hpp"
#include "dbgeth_pkt.hpp"

class packetreader {
public:
	packetreader(char* payload, uint32_t length)
	: m_header((dbgeth_pkt*) payload), m_current(0),
	  m_buffer((char*)(((dbgeth_pkt*) payload) + 1)),
	  m_length(length)
	{
		m_current = (dbgeth_pkt_query*)(m_header + 1);
		m_count = m_header->count;
	}

	~packetreader() {

	}

private:
	dbgeth_pkt* m_header;
	dbgeth_pkt_query* m_current;
	char* m_buffer;
	uint32_t m_length;
	uint32_t m_count;

public:
	dbgeth_pkt* header() {
		return m_header;
	}

	dbgeth_pkt_query* query() {
		return m_count > 0 ? m_current : 0;
	}

	bool next() {
		int len = 0;
		if(m_count <= 0)
			return false;

		m_count--;
		len = ntohs(m_current->length);
		m_current = (dbgeth_pkt_query*)((char*)(m_current + 1) + len);
		return true;
	}
};

#endif /* PROTOCOL_PACKETREADER_HPP_ */
