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
 * Implementation of GraphicId class.
 */

#include <DrawServ/drawlink.h>
#include <DrawServ/drawlinklist.h>
#include <DrawServ/drawserv.h>
#include <DrawServ/grid.h>
#include <DrawServ/gridlist.h>

#include <Unidraw/iterator.h>

#include <stdlib.h>
#include <time.h>

/*****************************************************************************/

GraphicId::GraphicId () 
{
  _id = DrawServ::candidate_grid();
  GraphicIdTable* table = ((DrawServ*)unidraw)->gridtable();
  table->insert(_id, this);
}

GraphicId::~GraphicId () 
{
  GraphicIdTable* table = ((DrawServ*)unidraw)->gridtable();
  table->remove(_id);
}

/*****************************************************************************/

GraphicIds::GraphicIds(GraphicId* subids, int nsubs) : GraphicId ()
{
  if (nsubs>0) {
    _sublist = new GraphicIdList();
     for(int i=0; i<nsubs; i++)
      _sublist->Append(&subids[i]);
  }
  Resource::ref(_sublist);
}

GraphicIds::GraphicIds(GraphicIdList* sublist) : GraphicId ()
{
  if (!sublist) sublist = new GraphicIdList;
  _sublist = sublist;
  _sublist->Reference();
}

GraphicIds::~GraphicIds () 
{
  Unref(_sublist);
  _sublist = nil;
}
