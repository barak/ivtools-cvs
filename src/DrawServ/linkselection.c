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
#include <DrawServ/gridlist.h>
#include <DrawServ/linkselection.h>

#include <OverlayUnidraw/ovcomps.h>
#include <OverlayUnidraw/ovviews.h>

#include <Unidraw/iterator.h>

#include <Attribute/attrlist.h>
#include <Attribute/attrvalue.h>

#include <stdio.h>

GraphicIdList* LinkSelection::_locally_selected = nil;
GraphicIdList* LinkSelection::_waiting_to_be_selected = nil;
char* LinkSelection::_selected_strings[] =  { "NotSelected", "LocallySelected", "RemotelySelected", "WaitingToBeSelected", "PreviouslySelected" };

/*****************************************************************************/

LinkSelection::LinkSelection (LinkSelection* s) : OverlaySelection(s) {
  if (!_locally_selected) {
    _locally_selected = new GraphicIdList;
    _waiting_to_be_selected = new GraphicIdList;
  }
}

LinkSelection::LinkSelection (Selection* s) : OverlaySelection(s) {
  if (!_locally_selected) {
    _locally_selected = new GraphicIdList;
    _waiting_to_be_selected = new GraphicIdList;
  }
}

void LinkSelection::Update(Viewer* viewer) {
  fprintf(stderr, "LinkSelection::Update\n");
  Reserve();
  OverlaySelection::Update(viewer);
}

void LinkSelection::Clear(Viewer* viewer) {
  fprintf(stderr, "LinkSelection::Clear\n");
  OverlaySelection::Clear(viewer);
}

void LinkSelection::Reserve() {
  fprintf(stderr, "LinkSelection::Reserve\n");
  static int id_sym = symbol_add("id");

  /* remove everything from the selected and waiting lists */
  Iterator it;
  _locally_selected->First(it);
  while (!_locally_selected->Done(it)) {
    GraphicId* grid = _locally_selected->GetGraphicId(it);
    _locally_selected->Remove(it);
    grid->selected(NotSelected);
  }
  _waiting_to_be_selected->First(it);
  while (!_waiting_to_be_selected->Done(it)) {
    GraphicId* grid = _waiting_to_be_selected->GetGraphicId(it);
    _waiting_to_be_selected->Remove(it);
    grid->selected(NotSelected);
  }

  /* remove anything from selection that has a remote selector */
  First(it);
  while (!Done(it)) {
    int removed = false;
    OverlayView* view = GetView(it);
    OverlayComp* comp = view ? view->GetOverlayComp() : nil;
    AttributeList* al = comp ? comp->attrlist() : nil;
    AttributeValue* av = al ? al->find(id_sym) : nil;
    if (av) {
      void* ptr = nil;
      ((DrawServ*)unidraw)->gridtable()->find(ptr, av->uint_val());
      if (ptr) {
	GraphicId* grid = (GraphicId*)ptr;
	if (grid->selector() && ((DrawServ*)unidraw)->sessionid()!=grid->selector()) {

	  /* make a request to select this in the future */
	  ((DrawServ*)unidraw)->ReserveSelection(grid);

	  Remove(it);
	  removed = true;

	  /* add to list of graphics waiting to be selected */
	  grid->selected(WaitingToBeSelected);
	  _waiting_to_be_selected->Append(grid);
	} else {

	  /* add to list of graphics currently selected */
	  grid->selected(LocallySelected);
	  grid->selector(((DrawServ*)unidraw)->sessionid());
	  _waiting_to_be_selected->Append(grid);
	}
      }
    }
    if (!removed) 
      Next(it);
  }
}
