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
 * Implementation of LinkSelection class.
 */

#include <DrawServ/drawserv.h>
#include <DrawServ/grid.h>
#include <DrawServ/linkselection.h>

#include <OverlayUnidraw/ovcomps.h>
#include <OverlayUnidraw/ovviews.h>

#include <Unidraw/iterator.h>

#include <Attribute/attrlist.h>
#include <Attribute/attrvalue.h>

#include <stdio.h>

/*****************************************************************************/

LinkSelection::LinkSelection (DrawLinkList* linklist, LinkSelection* s) : OverlaySelection(s) {
  _linklist = linklist;
}

LinkSelection::LinkSelection (DrawLinkList* linklist, Selection* s) : OverlaySelection(s) {
  _linklist = linklist;
}

int LinkSelection::Update(Viewer* viewer) {
  fprintf(stderr, "LinkSelection::Update\n");
  static int id_sym = symbol_add("id");

  /* remove anything from selection that has a remote selector */
  Iterator it;
  First(it);
  while (!Done(it)) {
    OverlayView* view = GetView(it);
    OverlayComp* comp = view ? view->GetOverlayComp() : nil;
    AttributeList* al = comp ? comp->attrlist() : nil;
    AttributeValue* av = al ? al->find(id_sym) : nil;
    if (av) {
      void* ptr = nil;
      ((DrawServ*)unidraw)->gridtable()->find(ptr, av->uint_val());
      if (ptr) {
	GraphicId* grid = (GraphicId*)ptr;
	if (grid->selector() && grid->sessionid()!=grid->selector())
	  Remove(it);
      }
    }
    Next(it);
  }
  return OverlaySelection::Update(viewer);
}

void LinkSelection::Clear(Viewer* viewer) {
  fprintf(stderr, "LinkSelection::Clear\n");
  OverlaySelection::Clear(viewer);
}

void LinkSelection::Reserve() {
  fprintf(stderr, "LinkSelection::Reserve\n");
}
