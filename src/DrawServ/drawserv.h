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
 * DrawServ - Unidraw derived from OverlayUnidraw for DrawServ library
 */
#ifndef drawserv_h
#define drawserv_h

#include <OverlayUnidraw/ovunidraw.h>
#include <stdio.h>

#include <OS/table.h>
declareTable(GraphicIdTable,int,void*)
declareTable(SessionIdTable,int,void*)

//: Unidraw specialized for DrawServ
// Unidraw (OverlayUnidraw) specialized for DrawServ application.
// Networked application of the Unidraw framework.

class AttributeValueList;
class ComTerp;
class DrawLink;
class DrawLinkList;
class GraphicId;
class GraphicIdList;

class DrawServ : public OverlayUnidraw {
public:
  DrawServ(
	   Catalog*, int& argc, char** argv, 
	   OptionDesc* = nil, PropertyData* = nil
	   );
  DrawServ(Catalog*, World*);
  virtual ~DrawServ();
  
  void Init();
  
  int online();
  // return true if session id initialized, otherwise false.
  
  DrawLink* linkup(const char* hostname, int portnum, 
		   int state, int local_id=-1, int remote_id=-1,
		   ComTerp* comterp=nil);
  // Create new link to remote drawserv, return -1 if error
  // state: 0==new_link, 1==one_way, 2==two_way.
  // Let DrawLink assign local_id by passing -1 for local_id.
  // The local_id argument is for verification purposes once
  // two-way link is established.
  
  int linkdown(DrawLink* link);
  // shut down existing link to remote drawserv
  
  DrawLink* linkget(int local_id, int remote_id=-1);
  // return pointer to existing DrawLink
  
  void linkdump(FILE*);
  // dump text table of DrawLink's
  
  virtual void ExecuteCmd(Command*);
  // execute Command's locally, and on remote linked DrawServ's.
  
  virtual void DistributeCmdString(const char* cmdstring);
  // execute command string remote DrawServ's
  
  virtual void SendCmdString(DrawLink* link, const char* cmdstring);
  // execute command string on one remote DrawServ

  virtual void ReserveSelection(GraphicId* grid);
  // send request to reserve graphic for future selection
  
  DrawLinkList* linklist() { return _linklist; }
  // return pointer to list of DrawLink's
  
  GraphicIdTable* gridtable() { return _gridtable; }
  // return pointer to table of GraphicId's.
  
  SessionIdTable* sessionidtable() { return _sessionidtable; }
  // return pointer to table of DrawLink's as a function of session id's.
  
  void sessionid_request_chk();
  // generate next request to check unique session id
  
  void sessionid_handle_chk(int chk_id, int remote_linkid);
  // handle request to check unique session id
  
  void sessionid_callback_chk(int chk_id, int remote_linkid, int ok_flag);
  // process callbacks on request to check unique session id
  
  unsigned int sessionid(int trial=false) { return trial ? _trialid : _sessionid; }
  // current unique session id.

  void reserve_handle(unsigned int id, unsigned int selector);
  // handle reserve request from remote DrawLink.

  void reserve_change(unsigned int id, unsigned int selector, boolean ok); 
  // callback for reserve request that goes to every DrawLink.

  boolean reserve_if_not_selected(GraphicId* grid, unsigned int selector);
  // check if graphic is selected, and allow it to be borrowed if not.
  
  static unsigned int candidate_grid();
  // generate candidate graphic id.
  static int unique_grid(unsigned int id);
  // test candidate graphic id for local uniqueness
  static unsigned int candidate_sessionid();
  // generate candidate session id.
  static int unique_sessionid(unsigned int id);
  // test candidate session id for local uniqueness
  
  static unsigned int GraphicIdMask;
  static unsigned int SessionIdMask;
  
protected:
    DrawLinkList* _linklist;
    // DrawLink list
    GraphicIdTable* _gridtable;
    // table of all GraphicId's.
    // maps from id to GraphicId*
    SessionIdTable* _sessionidtable;
    // table of all session id's.
    // maps from id to DrawLink*

    int _sessionid;
    // unique session id.
    int _trialid;
    // session id undergoing reservation

};

#endif
