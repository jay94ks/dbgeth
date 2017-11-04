/*
 * dbgeth_pkt.hpp
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

#ifndef PROTOCOL_DBGETH_PKT_HPP_
#define PROTOCOL_DBGETH_PKT_HPP_

#include <stdint.h>

enum dbgeth_query_types {
	E_DBGQRY_NOTHING = 0,
	E_DBGQRY_DISCOVERY,		// Somebody are there? / Are you okay?
	E_DBGQRY_NOTIFICATION,	// Yes, I'm here!
};

enum dbgeth_query_flags {
	E_DBGFLG_REQUEST		= 0x01,
	E_DBGFLG_RESPONSE		= 0x02,
	E_DBGFLG_REPEAT		= 0x04,
	E_DBGFLG_FINAL		= 0x08,
	E_DBGFLG_INDIVIDUAL	= 0x10 //individually
};

struct dbgeth_pkt_query {
	uint16_t	ident;
	uint8_t	type;
	uint8_t	flags;
	uint16_t	length;
	// below assignments will be contents of query (length).
};

struct dbgeth_pkt {
	uint32_t	magic;
	uint16_t	chkavg; // avg(ALL WORDS)
	uint16_t	version; // version.
	uint8_t	count; // query count
	// below things are filled by queries.
};

#define DBGETH_MAGIC		*((uint32_t*)"dbet")
#define DBGETH_VERSION	1			// 0.1 -> HIGH BYTE: MAJOR, LOW BYTE: MINOR

#endif /* PROTOCOL_DBGETH_PKT_HPP_ */
