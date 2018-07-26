/*
 * TCPStack.h
 *
 *                      The BSD License
 *
 *           Copyright (c) 2014, tomoaki@tomy-tech.com
 *                    All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  Created on: 2014/06/01
 *    Modified:
 *      Author: Tomoaki YAMAGUCHI
 *     Version: 0.0.0
 */

#ifndef TCPSTACK_H
#define TCPSTACK_H


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <arpa/inet.h>
#include "Defines.h"
#include "Messages.h"

#define SOCKET_MAXCONNECTIONS  5
#define SOCKET_MAXBUFFER_LENGTH 1280 // buffer size

using namespace std;

/*========================================
       Class TCPStack
 =======================================*/
class TCPStack{
public:
	TCPStack();
	virtual ~TCPStack();
	void disconnect();

	// Server initialization
	bool bind( const char* service );
	bool listen();
	bool accept( TCPStack& );

	// Client initialization
	bool connect( const char* host, const char* service );

	int send( const uint8_t* buf, uint16_t length );
	int  recv ( uint8_t* buf, uint16_t len );
	void close();

	void setNonBlocking( const bool );

	bool isValid();
	int getSock(){return _sockfd;}
private:
	int _sockfd;
	addrinfo* _addrinfo;
	Semaphore _sem;
	bool   _disconReq;

};



#endif /* TCPSTACK_H */
