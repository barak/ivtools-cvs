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
 * Implementation of Drawlink class.
 */

#include <DrawServ/drawlink.h>
#include <Unidraw/globals.h>
#include <fstream.h>
#include <unistd.h>

/*****************************************************************************/

DrawLink::DrawLink (const char* hostname, int portnum)
{
  _host = strnew(hostname);
  _port = portnum;
  _up = false;

  _addr = nil;
  _socket = nil;
  _conn = nil;

#if defined(HAVE_ACE) && (__GNUC__>3 || __GNUC__==3 && __GNUC_MINOR__>0)
  _addr = new ACE_INET_Addr(portnum, hostname);
  _socket = new ACE_SOCK_Stream;
  _conn = new ACE_SOCK_Connector;
  if (_conn->connect (*_socket, *_addr) == -1)
    ACE_ERROR ((LM_ERROR, "%p\n", "open"));
  else {

    fileptr_filebuf obuf(_socket->get_handle(), ios_base::out, false, static_cast<size_t>(BUFSIZ));
    ostream out(&obuf);
    out << "drawlink(\"";
    char buffer[HOST_NAME_MAX];
    gethostname(buffer, HOST_NAME_MAX);
    out << buffer;
    out << "\")\n";
    out.flush();
    _up = true;
  }
#else
  fprintf(stderr, "drawserv requires ACE and >= gcc-3.1 for full functionality\n");
#endif
}

DrawLink::~DrawLink () 
{
#ifdef HAVE_ACE
    if (_socket->close () == -1)
        ACE_ERROR ((LM_ERROR, "%p\n", "close"));
    delete _conn;
    delete _socket;
    delete _addr;
    delete _host;
#endif
}
