/*
 * Copyright (c) 2004 Scott E. Johnston
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of the copyright holders not be used in
 * advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.  The copyright holders make
 * no representations about the suitability of this software for any purpose.
 * It is provided "as is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
 * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 */

/*
 * DrawLink - object to encapsulate 2-way link with remote drawserv
 */
#ifndef drawlink_h
#define drawlink_h

class DrawServ;

#ifdef HAVE_ACE
#include <ComTerp/comhandler.h>
#include <ace/SOCK_Connector.h>

class ACE_INET_Addr;
class ACE_SOCK_Stream;
class ACE_SOCK_Stream;
#endif

//: object to encapsulate 2-way link with remote drawserv
class DrawLink {
public:
    DrawLink(const char* hostname, int portnum);
    virtual ~DrawLink();

    const char* hostname() { return _host; }
    // return name of remote host

    int portnum() { return _port; }
    // return port on remote host

    int up() { return _up; }
    // return 1 if link up, 0 if down

protected:
    const char* _host;
    int _port;
    int _up;

#ifdef HAVE_ACE
    ACE_INET_Addr* _addr;
    ACE_SOCK_Connector* _conn;
    ACE_SOCK_Stream* _socket;
#endif
};

#endif
