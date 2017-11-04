/*
 * packetbuilder.hpp
 *
 *  Created on: 2017. 11. 4.
 *      Author: jaehoon
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

#ifndef PROTOCOL_PACKETBUILDER_HPP_
#define PROTOCOL_PACKETBUILDER_HPP_

#include <stdint.h>
#include <stdlib.h>
#include "rawsock_t.hpp"
#include "dbgeth_pkt.hpp"

class packetbuilder {
public:
	packetbuilder()
	: m_buffer(new char[sizeof(dbgeth_pkt)]),
	  m_allocs(sizeof(dbgeth_pkt)), m_header(0)
	{
		m_header = (dbgeth_pkt*) m_buffer;

		memset(m_buffer, 0, m_allocs);

		m_header->magic = DBGETH_MAGIC;
		m_header->version = htons(DBGETH_VERSION);
		m_header->chkavg = 0;
		m_header->count = 0;
	}

	~packetbuilder() {
		if(m_buffer)
			delete[] (m_buffer);

		m_buffer = 0;
	}

private:
	char* m_buffer;
	uint32_t m_allocs;

private:
	dbgeth_pkt* m_header;

public:
	void* buffer() {
		return m_buffer;
	}

	uint32_t length() {
		return m_allocs;
	}

	void calc_chkavg() {
		uint16_t k = 0;

		m_header->chkavg = 0;
		for(uint32_t i = 0; i < m_allocs / 2; i++)
			k = (k + ((uint16_t*) m_buffer)[i]) / 2;

		m_header->chkavg = htons(k);
	}

public:
	void add_query(uint16_t ident, uint8_t type,
		uint8_t flags, void* contents, uint32_t length)
	{
		char* buffer = new char[sizeof(dbgeth_pkt_query) + length];
		dbgeth_pkt_query* query = (dbgeth_pkt_query*) buffer;

		query->ident = htons(ident);
		query->type = type;
		query->flags = flags;
		query->length = htons(length = contents ? length : 0);

		if(contents)
			memcpy(query + 1, contents, length);

		{
			char* newbie = new char[m_allocs + (sizeof(dbgeth_pkt_query) + length)];
			memcpy(newbie, m_buffer, m_allocs);
			memcpy(newbie + m_allocs, buffer, (sizeof(dbgeth_pkt_query) + length));
			delete[] m_buffer;

			m_buffer = newbie;
			m_allocs = m_allocs + (sizeof(dbgeth_pkt_query) + length);

			m_header = (dbgeth_pkt*) m_buffer;
			m_header->count = m_header->count + 1;
		}

		delete[] buffer;
	}
};


#endif /* PROTOCOL_PACKETBUILDER_HPP_ */
