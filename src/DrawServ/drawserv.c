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
 * Implementation of DrawServ class.
 */

#include <DrawServ/drawlink.h>
#include <DrawServ/drawlinklist.h>
#include <DrawServ/drawserv.h>

#include <Unidraw/iterator.h>
#include <Unidraw/ulist.h>
/*****************************************************************************/

DrawServ::DrawServ (Catalog* c, int& argc, char** argv, 
		    OptionDesc* od, PropertyData* pd) 
: OverlayUnidraw(c, argc, argv, od, pd) {
  Init();
}

DrawServ::DrawServ (Catalog* c, World* w) 
: OverlayUnidraw(c, w) {
  Init();
}

void DrawServ::Init() {
  _list = new DrawLinkList;
}

DrawServ::~DrawServ () 
{
}

int DrawServ::linkup(const char* hostname, int portnum, 
		     int state, int local_id, int remote_id) {
  if (state == DrawLink::new_link || state == DrawLink::one_way) {
    DrawLink* link = new DrawLink(hostname, portnum, state);
    if (link && link->ok()) {
      link->remote_linkid(remote_id);
      _list->add_drawlink(link);
      return 0;
    } else {
      delete link;
      return -1;
    }
  } else {
    // search for existing link with matching local_id
    Iterator i;
    _list->First(i);
    while(!_list->Done(i) && _list->GetDrawLink(i)->local_linkid()!=local_id);
    if (!_list->Done(i)) {
      DrawLink* curlink = _list->GetDrawLink(i);
      curlink->remote_linkid(remote_id);
      fprintf(stderr, "link up with %s via port %d\n", hostname, portnum);
      fprintf(stderr, "local id %d, remote id %d\n", curlink->local_linkid(), 
	      curlink->remote_linkid());
    } else
      fprintf(stderr, "unable to complete two-way link\n");
  }
}

  
