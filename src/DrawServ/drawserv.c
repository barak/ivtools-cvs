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

#include <DrawServ/draweditor.h>
#include <DrawServ/drawlink.h>
#include <DrawServ/drawlinklist.h>
#include <DrawServ/drawserv.h>
#include <DrawServ/drawserv-handler.h>
#include <DrawServ/grid.h>
#include <DrawServ/gridlist.h>

#include <OverlayUnidraw/ovclasses.h>
#include <OverlayUnidraw/scriptview.h>

#include <Unidraw/Commands/command.h>
#include <Unidraw/catalog.h>
#include <Unidraw/clipboard.h>
#include <Unidraw/creator.h>
#include <Unidraw/iterator.h>
#include <Unidraw/ulist.h>

#include <ComTerp/comterpserv.h>

#include <Attribute/attrlist.h>

#include <fstream.h>
#include <strstream>

int DrawServ::_reserve_batch_min = 3;
int DrawServ::_reserve_batch_size = 5;

implementTable(GraphicIdTable,int,void*)

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
  _linklist = new DrawLinkList;
  _grid_request_list = new GraphicIdList;
  _grid_reserved_list = new GraphicIdList;
  _gridtable = new GraphicIdTable(1024);
}

DrawServ::~DrawServ () 
{
  delete _linklist;
  delete _grid_request_list;
  delete _grid_reserved_list;
  delete _gridtable;
}

DrawLink* DrawServ::linkup(const char* hostname, int portnum, 
		     int state, int local_id, int remote_id,
		     ComTerp* comterp) {
  if (state == DrawLink::new_link || state == DrawLink::one_way) {
    DrawLink* link = new DrawLink(hostname, portnum, state);
    link->remote_linkid(remote_id);
    if (state==DrawLink::one_way && comterp && comterp->handler()) {
      ((DrawServHandler*)comterp->handler())->drawlink(link);
      link->handler((DrawServHandler*)comterp->handler());
    }
    link->open();
    if (link && link->ok()) {
      _linklist->add_drawlink(link);
      return link;
    } else {
      delete link;
      return nil;
    }
  } else {

    // search for existing link with matching local_id
    Iterator i;
    _linklist->First(i);
    while(!_linklist->Done(i) && _linklist->GetDrawLink(i)->local_linkid()!=local_id)
      _linklist->Next(i);

    /* if found, finalize linkup */
    if (!_linklist->Done(i)) {
      DrawLink* curlink = _linklist->GetDrawLink(i);
      curlink->remote_linkid(remote_id);
      curlink->althostname(hostname);
      if (comterp && comterp->handler()) {
	((DrawServHandler*)comterp->handler())->drawlink(curlink);
	curlink->handler((DrawServHandler*)comterp->handler());
      }
      fprintf(stderr, "link up with %s(%s) via port %d\n", 
	      curlink->hostname(), curlink->althostname(), portnum);
      fprintf(stderr, "local id %d, remote id %d\n", curlink->local_linkid(), 
	      curlink->remote_linkid());
      return curlink;
    } else {
      fprintf(stderr, "unable to complete two-way link\n");
      return nil;
    }
  }
}

int DrawServ::linkdown(DrawLink* link) {
  if (_linklist->Includes(link)) {
    _linklist->Remove(link);
    link->close();
    delete link;
    return 0;
  } else
    return -1;
}

DrawLink* DrawServ::linkget(int local_id, int remote_id) {
  DrawLink* link = nil;
  if (_linklist) {
    Iterator(i);
    _linklist->First(i);
    while (!_linklist->Done(i) && !link) {
      DrawLink* l = _linklist->GetDrawLink(i);
      if (l->local_linkid()==local_id && remote_id==-1 ||
	  local_id==-1 && l->remote_linkid()==remote_id ||
	  l->local_linkid()==local_id && l->remote_linkid()==remote_id)
	link = l;
      _linklist->Next(i);
    }
  }
  return link;
}

void DrawServ::linkdump(FILE* fptr) {
  fprintf(fptr, "Host                            Alt.                            Port    LID  RID\n");
  fprintf(fptr, "------------------------------  ------------------------------  ------  ---  ---\n");
  if (_linklist) {
    Iterator i;
    _linklist->First(i);
    while(!_linklist->Done(i)) {
      DrawLink* link = _linklist->GetDrawLink(i);
      fprintf(fptr, "%-30.30s  %-30.30s  %-6d  %-3d  %-3d\n", 
	      link->hostname(), link->althostname(), link->portnum(),
	      link->local_linkid(), link->remote_linkid());
      _linklist->Next(i);
    }
  }
}

void DrawServ::ExecuteCmd(Command* cmd) {
  if(!_linklist || _linklist->Number()==0) 

    /* normal Unidraw command execution */
    Unidraw::ExecuteCmd(cmd);

  else {

    /* indirect command execution, all by script */
    std::ostrstream sbuf;
    boolean oldflag = OverlayScript::ptlist_parens();
    OverlayScript::ptlist_parens(false);
    switch (cmd->GetClassId()) {
    case PASTE_CMD:
      {
      boolean scripted = false;
      Clipboard* cb = cmd->GetClipboard();
      if (cb) {
	Iterator it;
	for (cb->First(it); !cb->Done(it); cb->Next(it)) {
	  OverlayComp* comp = (OverlayComp*)cb->GetComp(it);
	  if (comp) {
	    Creator* creator = unidraw->GetCatalog()->GetCreator();
	    OverlayScript* scripter = (OverlayScript*)
	      creator->Create(Combine(comp->GetClassId(), SCRIPT_VIEW));
	    if (scripter) {
	      scripter->SetSubject(comp);
	      if (scripted) 
		sbuf << ';';
	      else 
		scripted = true;
	      boolean status = scripter->Definition(sbuf);
	      delete scripter;
	    }
	  }
	}
      }
      if (!scripted)
	sbuf << "print(\"Failed attempt to generate script for a PASTE_CMD\\n\" :err)";
      sbuf.put('\0');
      cout << sbuf.str() << "\n";
      cout.flush();

      /* first execute here */
      ((ComEditor*)cmd->GetEditor())->GetComTerp()->run(sbuf.str());

      /* then send everywhere else */
      DistributeCmdString(sbuf.str());

      delete cmd;
      }
      break;
    default:
      sbuf << "print(\"Attempt to convert unknown command (id == %d) to interpretable script\\n\" " << cmd->GetClassId() << " :err)";
      cmd->Execute();
      if (cmd->Reversible()) {
	cmd->Log();
      } else {
	delete cmd;
      }
      break;
    }
    OverlayScript::ptlist_parens(oldflag);
  }
}

void DrawServ::DistributeCmdString(const char* cmdstring) {

  Iterator i;
  _linklist->First(i);
  while (!_linklist->Done(i)) {
    DrawLink* link = _linklist->GetDrawLink(i);
    if (link) {
      int fd = link->handle();
      if (fd>=0) {
	fileptr_filebuf fbuf(fd, ios_base::out, false, static_cast<size_t>(BUFSIZ));
	ostream out(&fbuf);
	out << cmdstring;
	out << "\n";
	out.flush();
      }
    }
    _linklist->Next(i);
  }

}

void DrawServ::reserve_batch_request(int nids) {

  /* generate a new batch of id's while generating command to reserve them*/
  GraphicIdsRequest* gridr = new GraphicIdsRequest();
  GraphicIdList* gridr_list = gridr->sublist();
  std::ostrstream sbuf;
  sbuf << "reserve(";
  for (int i=0; i<_reserve_batch_size; i++) {
    GraphicId* grid = new GraphicId();
    gridr_list->Append(grid);
    if (i) sbuf << ',';
    sbuf << grid->id();
  }
  sbuf << ":batchid " << gridr->id() << ")";
  DistributeCmdString(sbuf.str());

}

void DrawServ::reserve_batch_response(int reserved, unsigned int batchid) {
}

void DrawServ::reserve_batch_handle(DrawLink* link, unsigned int *ids, int nids, unsigned int batchid) {
  fprintf(stderr, "reserve batch request has been made, batchid==%d\n, batchid");
  for (int i=0; i<nids; i++) {
    if (i) fprintf(stderr, ",");
    fprintf(stderr, "%d", ids[i]);
  }
  fprintf(stderr, "\n");
}

int DrawServ::reserved_id() {
  int return_id = 0;

  /* process next batch if no more id's ready */
  if (_grid_reserved_list->Number()==0) reserved_batch();

  /* reserve a new batch if number available falls below a threshold */
  if (_grid_request_list->Number()<reserve_batch_min()) reserve_batch_request(reserve_batch_size());

  return return_id;
}

int DrawServ::reserved_batch() {

  int count = 0;

  /* search through list of candidate id sets, and remove 1st */
  /* set that has completed the reservation process           */
  Iterator it;
  _grid_request_list->First(it);
  GraphicIdsRequest* gridr = nil;
  while (!_grid_request_list->Done(it) && !gridr) {
    GraphicIdsRequest* gridr = (GraphicIdsRequest*)_grid_request_list->GetGraphicId(it);
    if(gridr && gridr->reserved()) {
      _grid_request_list->Remove(it);
    }
    _grid_request_list->Next(it);
  }

  /* transfer all of these to the reservation completed list */
  if (gridr && gridr->sublist()) {
    Iterator jt;
    GraphicIdList* gridr_list = gridr->sublist();
    count = gridr_list->Number();
    gridr_list->First(jt);
    while (!gridr_list->Done(jt)) {
      GraphicId* grid = gridr_list->GetGraphicId(jt);
      gridr_list->Remove(jt);
      if (grid) _grid_reserved_list->Append(grid);
    }
    delete gridr;
    return count;
  } else {
    fprintf(stderr, "No batch of graphic ids has completed reservation process\n");
    fprintf(stderr, "Someone should really do something about this\n");
    return 0;
  }
}

