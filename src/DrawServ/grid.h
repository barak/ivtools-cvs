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
 * GraphicId - object to encapsulate 2-way link with remote drawserv
 */
#ifndef grid_h
#define grid_h

#include <Unidraw/globals.h>

class DrawLink;
class GraphicIdList;

//: object to encapsulate unique graphic id
class GraphicId {
public:
  GraphicId();
  virtual ~GraphicId();
  
  unsigned int id() { return _id; }
  // get associated unique integer id

  virtual int composite() { return 0; }
  // return true if can be cast to GraphicIds

  virtual GraphicIdList* sublist() { return nil; }
  // returns pointer to sublist of GraphicId's, if any

  static int candidate_grid();
  // generate candidate graphic id.
  static int unique_grid(unsigned int id);
  // test candidate graphic id for local uniqueness
  static int candidate_sessionid();
  // generate candidate session id.
  static int unique_sessionid(unsigned int id);
  // test candidate session id for local uniqueness
  
  static unsigned int GraphicIdMask;
  static unsigned int SessionIdMask;
  
protected:
  unsigned int _id;

};

//: object to encapsulate a set of graphic ids
class GraphicIds : public GraphicId {
public:
  GraphicIds(GraphicId* subids, int nsubids); 
  GraphicIds(GraphicIdList* sublist=nil);
  virtual ~GraphicIds();

  virtual int composite() { return 1; }
  // return true if can be cast to GraphicIds
  virtual GraphicIdList* sublist() { return _sublist; }
  // returns pointer to sublist of GraphicId's, if any

protected:
  GraphicIdList* _sublist;
};

#endif
