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

int DrawLink::_linkcnt = 0;

implementTable(IncomingSidTable,unsigned int,unsigned int)

/*****************************************************************************/

DrawLink::DrawLink (const char* hostname, int portnum, int state)
{
  _host = strnew(hostname);
  _althost = nil;
  _port = portnum;
  _ok = false;
  _local_linkid = _linkcnt++;
  _remote_linkid = -1;
  _state = state;

  _addr = nil;
  _socket = nil;
  _conn = nil;

  _handler = nil;
  _incomingsidtable = new IncomingSidTable(32);
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
    delete _althost;
#endif
    delete _incomingsidtable;
}

int DrawLink::open() {

#if defined(HAVE_ACE) && (__GNUC__>3 || __GNUC__==3 && __GNUC_MINOR__>0)
  _addr = new ACE_INET_Addr(_port, _host);
  _socket = new ACE_SOCK_Stream;
  _conn = new ACE_SOCK_Connector;
  if (_conn->connect (*_socket, *_addr) == -1) {
    ACE_ERROR ((LM_ERROR, "%p\n", "open"));
    return -1;
  } else {
    fileptr_filebuf obuf(_socket->get_handle(), ios_base::out, false, static_cast<size_t>(BUFSIZ));
    ostream out(&obuf);
    out << "drawlink(\"";
    char buffer[HOST_NAME_MAX];
    gethostname(buffer, HOST_NAME_MAX);
    out << buffer << "\"";
    out << " :state " << _state+1;
    out << " :rid " << _local_linkid;
    out << " :lid " << _remote_linkid;
    out << ")\n";
    out.flush();
    _ok = true;
    return 0;
  }
#else
  fprintf(stderr, "drawserv requires ACE and >= gcc-3.1 for full functionality\n");
  return -1;
#endif
}

int DrawLink::close() {
  fprintf(stderr, "Closing link to %s (%s) port # %d (lid=%d, rid=%d)\n", 
	  hostname(), althostname(), portnum(), local_linkid(), remote_linkid());
  if (_socket) {
    if (_socket->close () == -1)
      ACE_ERROR ((LM_ERROR, "%p\n", "close"));
  }
}

void DrawLink::hostname(const char* host) {
  delete _host;
  _host = nil;
  if (host) _host = strnew(host);
}

void DrawLink::althostname(const char* althost) {
  delete _althost;
  _althost = nil;
  if (althost) _althost = strnew(althost);
}

int DrawLink::handle() {
  if (_socket) return _socket->get_handle();
  else return -1;
}
