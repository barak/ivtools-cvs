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
#include <DrawServ/linkselection.h>

#include <OverlayUnidraw/ovviews.h>

#include <Unidraw/iterator.h>

#include <stdio.h>

/*****************************************************************************/

LinkSelection::LinkSelection (DrawLinkList* linklist, LinkSelection* s) : OverlaySelection(s) {
  _linklist = linklist;
}

LinkSelection::LinkSelection (DrawLinkList* linklist, Selection* s) : OverlaySelection(s) {
  _linklist = linklist;
}

void LinkSelection::Merge (Selection* s) {
  fprintf(stderr, "LinkSelection::Merge\n");
  OverlaySelection::Merge(s);
}

void LinkSelection::Exclusive (Selection* s) {
  fprintf(stderr, "LinkSelection::Exclusive\n");
  OverlaySelection::Exclusive(s);
}

void LinkSelection::Append (GraphicView* v) {
  fprintf(stderr, "LinkSelection::Append\n");
  OverlaySelection::Append(v);
}

void LinkSelection::Prepend (GraphicView* v) {
  fprintf(stderr, "LinkSelection::Prepend\n");
  OverlaySelection::Prepend(v);
}

void LinkSelection::InsertAfter (Iterator i, GraphicView* v) {
  fprintf(stderr, "LinkSelection::InsertAfter\n");
  OverlaySelection::InsertAfter(i, v);
}

void LinkSelection::InsertBefore (Iterator i, GraphicView* v) {
  fprintf(stderr, "LinkSelection::InsertBefore\n");
  OverlaySelection::InsertBefore(i, v);
}

void LinkSelection::Remove (Iterator& i) {
  fprintf(stderr, "LinkSelection::Remove(i)\n");
  OverlaySelection::Remove(i);
}	
    
void LinkSelection::Remove (GraphicView* p) {
  fprintf(stderr, "LinkSelection::Remove(p)\n");
  OverlaySelection::Remove(p);
}


OverlaySelection* LinkSelection::ViewsWithin(IntCoord l, IntCoord b, IntCoord r, IntCoord t) {
    OverlaySelection* newSel = new LinkSelection(((DrawServ*)unidraw)->linklist());
    Iterator i;
    for (First(i); !Done(i); Next(i)) {
	OverlayView* view = GetView(i);
	newSel->Merge(view->ViewsWithin(l, b, r, t));
    }
    return newSel;
}
