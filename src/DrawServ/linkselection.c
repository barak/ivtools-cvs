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
  CompIdTable* table = ((DrawServ*)unidraw)->compidtable();
  Iterator it;
  First(it);
  while(!Done(it)) {
    OverlayView* view = GetView(it);
    OverlayComp* comp = view ? view->GetOverlayComp() : nil;
    void* ptr = nil;
    table->find(ptr, (void*)comp);
    if (ptr) {
      GraphicId* grid = (GraphicId*)grid;
      if (grid->selected()==LocallySelected)
	grid->selected(PreviouslySelected);
      else if (grid->selected()==WaitingToBeSelected)
	grid->selected(NotSelected);
    }
    Next(it);
  }
  OverlaySelection::Clear(viewer);
  
}

void LinkSelection::Reserve() {
  fprintf(stderr, "LinkSelection::Reserve\n");
  static int id_sym = symbol_add("id");
  CompIdTable* table = ((DrawServ*)unidraw)->compidtable();

  /* remove anything from selection that has a remote selector */
  Iterator it;
  First(it);
  while (!Done(it)) {
    int removed = false;
    OverlayView* view = GetView(it);
    OverlayComp* comp = view ? view->GetOverlayComp() : nil;
#if 0
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

	  grid->selected(WaitingToBeSelected);
	} else {
	  grid->selected(LocallySelected);
	}
      }
    }
#else
    void* ptr = nil;
    table->find(ptr, (void*)comp);
    if (ptr) {
      GraphicId* grid = (GraphicId*)ptr;
      if (grid->selector() && ((DrawServ*)unidraw)->sessionid()!=grid->selector()) {
	
	/* make a request to select this in the future */
	((DrawServ*)unidraw)->ReserveSelection(grid);
	
	Remove(it);
	removed = true;
	
	grid->selected(WaitingToBeSelected);
      } else {
	grid->selected(LocallySelected);
      }
      
    }
#endif
    if (!removed) 
      Next(it);
  }
}
