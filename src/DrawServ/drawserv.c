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
#include <Unidraw/Commands/edit.h>
#include <Unidraw/catalog.h>
#include <Unidraw/clipboard.h>
#include <Unidraw/creator.h>
#include <Unidraw/iterator.h>
#include <Unidraw/ulist.h>

#include <ComTerp/comterpserv.h>

#include <Attribute/attrlist.h>
#include <Attribute/attrvalue.h>

#include <fstream.h>
#include <strstream>

implementTable(GraphicIdTable,int,void*)
implementTable(SessionIdTable,int,void*)

unsigned int DrawServ::GraphicIdMask = 0x000fffff;
unsigned int DrawServ::SessionIdMask = 0xfff00000;

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

  _gridtable = new GraphicIdTable(1024);
  _sessionidtable = new SessionIdTable(256);

  _sessionid = candidate_sessionid();
  _trialid = _sessionid;
  _sessionidtable->insert(_sessionid, nil); // nil pointer refers to local session id
}

DrawServ::~DrawServ () 
{
  delete _linklist;
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
  static int id_sym = symbol_add("id");
  static int sid_sym = symbol_add("sid");

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
	  AttributeList* al = comp->GetAttributeList();
	  AttributeValue* idv = al->find(id_sym);
	  AttributeValue* sidv = al->find(sid_sym);
	  
	  /* unique id already remotely assigned */
	  if (idv && idv->uint_val() !=0 && sidv && sidv->uint_val() !=0) {
	    GraphicId* grid = new GraphicId();
	    grid->id(idv->uint_val());
	    grid->selector(sidv->uint_val());
	  } 
	  
	  /* generate unique id and add as attribute */
	  /* also mark with selector id */
	  else {
	    GraphicId* grid = new GraphicId(sessionid());
	    grid->grcomp(comp);
	    grid->selector(((DrawServ*)unidraw)->sessionid());
	    AttributeValue* idv = new AttributeValue(grid->id(), AttributeValue::UIntType);
	    idv->state(AttributeValue::HexState);
	    al->add_attr(id_sym, idv);
	    AttributeValue* sidv = new AttributeValue(grid->selector(), AttributeValue::UIntType);
	    sidv->state(AttributeValue::HexState);
	    al->add_attr(sid_sym, sidv);
	  }
	    
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
#if 0
      ((ComEditor*)cmd->GetEditor())->GetComTerp()->run(sbuf.str());
      ((PasteCmd*)cmd)->executed(true);
#else
      cmd->Execute();
#endif

      /* then send everywhere else */
      DistributeCmdString(sbuf.str());
      
      }
      break;
    default:
      sbuf << "print(\"Attempt to convert unknown command (id == %d) to interpretable script\\n\" " << cmd->GetClassId() << " :err)";
      cmd->Execute();
      break;
    }

    if (cmd->Reversible()) {
      cmd->Log();
    } else {
      delete cmd;
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

void DrawServ::SendCmdString(DrawLink* link, const char* cmdstring) {

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
}

// generate request to check unique session id
void DrawServ::sessionid_request_chk() {

  Iterator it;
  _linklist->First(it);
  char buf[BUFSIZ];
  while(!_linklist->Done(it)) {
    DrawLink* link = _linklist->GetDrawLink(it);
    link->sessionid_state(DrawLink::SessionIdRequested);
    snprintf(buf, BUFSIZ, "sessionid(0x%08x :rid %d)%c", _trialid, link->local_linkid(), '\0');
    SendCmdString(link, buf);
    _linklist->Next(it);
  }
}

// handle request to check unique session id
// can be answered locally, because others are required to
// propogate their request completely before adopting new id.
void DrawServ::sessionid_handle_chk(int new_id, int remote_linkid) {
  DrawLink* link = linkget(-1, remote_linkid);
  if (link) {
    if (_linklist->Number()==1) {
      int okflag = false;
      if (okflag = DrawServ::unique_sessionid(new_id)) {
	SessionIdTable* table = ((DrawServ*)unidraw)->sessionidtable();
	table->insert(new_id, link);
      }
      char buf[BUFSIZ];
      snprintf(buf, BUFSIZ, "sessionid(0x%08x :ok %d :rid %d )%c", new_id, okflag, link->local_linkid(), '\0');
      SendCmdString(link, buf);
    } else {
      fprintf(stderr, "code for passing selection id request on to other DrawLink's not implemented\n");
    }
  } else
    fprintf(stderr, "no link with remote id of %d found\n", remote_linkid);
}

// process callbacks on request to check unique session id
void DrawServ::sessionid_callback_chk(int new_id, int remote_linkid, int ok_flag) {
  fprintf(stderr, "sessionid_callback_chk: new_id %d,  remote_linkid %d, ok_flag %d\n",
	  new_id, remote_linkid, ok_flag);
  if (new_id != _trialid) {
    ok_flag = false;
    fprintf(stderr, "new_id (%d) does not match _trialid (%d)\n", new_id, _trialid);
  }
  if (ok_flag) {
    _sessionid = _trialid;
  } else {
  }
}

// generate request to change/set unique session id
void DrawServ::sessionid_request_chg() {
}

// handle request to change unique session id
void DrawServ::sessionid_handle_chg(int new_id, int old_id) {
}

int DrawServ::online() {
  return _sessionid==_trialid && _sessionid!=-1;
}

unsigned int DrawServ::candidate_grid() {
  static int seed=0;
  if (!seed) {
    seed = time(nil) & (time(nil) << 16);
    srand(seed);
  }
  int retval;
  do {
    static int flip=0;
    while ((retval=rand()&GraphicIdMask)==0);

  } while (!unique_grid(retval));
  return retval;
}

int DrawServ::unique_grid(unsigned int id) {
  GraphicIdTable* table = ((DrawServ*)unidraw)->gridtable();
  void* ptr = nil;
  table->find(ptr, id);
  if (ptr) 
    return 0;
  else
    return 1;
}

unsigned int DrawServ::candidate_sessionid() {
  static int seed=0;
  if (!seed) {
    seed = time(nil) & (time(nil) << 16);
    srand(seed);
  }
  int retval;
  do {
    static int flip=0;
    while ((retval=rand()&SessionIdMask)==0);

  } while (!unique_sessionid(retval));
  return retval;
}

int DrawServ::unique_sessionid(unsigned int id) {
  SessionIdTable* table = ((DrawServ*)unidraw)->sessionidtable();
  void* ptr = nil;
  table->find(ptr, id);
  if (ptr) 
    return 0;
  else
    return 1;
}

