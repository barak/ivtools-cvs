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
#include <DrawServ/linkselection.h>

#include <OverlayUnidraw/ovclasses.h>
#include <OverlayUnidraw/ovviews.h>
#include <OverlayUnidraw/scriptview.h>

#include <Unidraw/Commands/command.h>
#include <Unidraw/Commands/edit.h>
#include <Unidraw/catalog.h>
#include <Unidraw/clipboard.h>
#include <Unidraw/creator.h>
#include <Unidraw/iterator.h>
#include <Unidraw/selection.h>
#include <Unidraw/ulist.h>
#include <Unidraw/viewer.h>

#include <ComTerp/comterpserv.h>

#include <Attribute/attrlist.h>
#include <Attribute/attrvalue.h>

#include <fstream.h>
#include <strstream>

implementTable(GraphicIdTable,int,void*)
implementTable(SessionIdTable,int,void*)
implementTable(CompIdTable,void*,void*)
implementTable(OriginalSidTable,unsigned int,unsigned int)

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
  _compidtable = new CompIdTable(1024);
  _originalsidtable = new OriginalSidTable(32);

  _sessionid = unique_sessionid();
  _sessionidtable->insert(_sessionid, nil); // nil pointer refers to local session id
}

DrawServ::~DrawServ () 
{
  delete _linklist;
  delete _gridtable;
  delete _sessionidtable;
  delete _compidtable;
  delete _originalsidtable;
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
      curlink->state(DrawLink::two_way);
      if (comterp && comterp->handler()) {
	((DrawServHandler*)comterp->handler())->drawlink(curlink);
	curlink->handler((DrawServHandler*)comterp->handler());
      }
      fprintf(stderr, "link up with %s(%s) via port %d\n", 
	      curlink->hostname(), curlink->althostname(), portnum);
      fprintf(stderr, "local id %d, remote id %d\n", curlink->local_linkid(), 
	      curlink->remote_linkid());

      /* register all sessionid's with other DrawServ */
      sessionid_register(curlink);
      SendCmdString(curlink, "sid(:all)");
      SendCmdString(curlink, "drawlink(:state 2)");

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
  boolean original = false;

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
	    grid->grcomp(comp);
	    grid->id(idv->uint_val());
	    grid->selector(sidv->uint_val());
	    grid->selected(LinkSelection::RemotelySelected);
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
	    original = true;
	  }
	    

	  if (comp&&original) {
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
      if (original) {
	if (!scripted)
	  sbuf << "print(\"Failed attempt to generate script for a PASTE_CMD\\n\" :err)";
	sbuf.put('\0');
	cout << sbuf.str() << "\n";
	cout.flush();
      }

      /* first execute here */
#if 0
      ((ComEditor*)cmd->GetEditor())->GetComTerp()->run(sbuf.str());
      ((PasteCmd*)cmd)->executed(true);
#else
      cmd->Execute();
#endif

      /* then send everywhere else */
      if (original) DistributeCmdString(sbuf.str());
      
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

// generate request to register each locally unique session id
void DrawServ::sessionid_register(DrawLink* link) {
  char buf[BUFSIZ];
  snprintf(buf, BUFSIZ, "sid(0x%08x 0x%08x)%c", sessionid(), sessionid(), '\0');
  SendCmdString(link, buf);
  SessionIdTable* table = ((DrawServ*)unidraw)->sessionidtable();
  SessionIdTable_Iterator it(*table);
  while(it.more()) {
    if(it.cur_value()) {
      DrawLink* otherlink = (DrawLink*)it.cur_value();
      if (otherlink != link) {
	unsigned int sid, osid;
	sid = (unsigned int)it.cur_key();
	originalsidtable()->find(osid, sid);
	snprintf(buf, BUFSIZ, "sid(0x%08x 0x%08x)%c", sid, osid, '\0');
	SendCmdString(link, buf);
      }
    }
    it.next();
  }
}

// handle request to register session id
void DrawServ::sessionid_register_handle(DrawLink* link, unsigned int sid, unsigned osid) {
  if (link) {
    SessionIdTable* sidtable = ((DrawServ*)unidraw)->sessionidtable();
    unsigned int alt_id = sid;
    if (!DrawServ::test_sessionid(sid)) 
      alt_id = DrawServ::unique_sessionid();
    sidtable->insert(alt_id, link);
    link->incomingsidtable()->insert(sid, alt_id);
    ((DrawServ*)unidraw)->originalsidtable()->insert(alt_id, osid);

    /* propagate */
 }

 }

unsigned int DrawServ::unique_grid() {
  static int seed=0;
  if (!seed) {
    seed = time(nil) & (time(nil) << 16);
    srand(seed);
  }
  int retval;
  do {
    static int flip=0;
    while ((retval=rand()&GraphicIdMask)==0);

  } while (!test_grid(retval));
  return retval;
}

int DrawServ::test_grid(unsigned int id) {
  GraphicIdTable* table = ((DrawServ*)unidraw)->gridtable();
  void* ptr = nil;
  table->find(ptr, id);
  if (ptr) 
    return 0;
  else
    return 1;
}

unsigned int DrawServ::unique_sessionid() {
  static int seed=0;
  if (!seed) {
    seed = time(nil) & (time(nil) << 16);
    srand(seed);
  }
  int retval;
  do {
    static int flip=0;
    while ((retval=rand()&SessionIdMask)==0);

  } while (!test_sessionid(retval));
  return retval;
}

int DrawServ::test_sessionid(unsigned int id) {
  SessionIdTable* table = ((DrawServ*)unidraw)->sessionidtable();
  void* ptr = nil;
  table->find(ptr, id);
  if (ptr) 
    return 0;
  else
    return 1;
}

void DrawServ::grid_message(GraphicId* grid) {
  char buf[BUFSIZ];
  if (grid->selected()==LinkSelection::LocallySelected) {
    snprintf(buf, BUFSIZ, "grid(0x%08x 0x%08x %d )%c", grid->id(), grid->selector(), 
	     LinkSelection::RemotelySelected, '\0');
    DistributeCmdString(buf);
  } else {
    
    /* find link on which current selector lives */
    DrawLink* link = _linklist->find_drawlink(grid);
    
    if (link) {
      snprintf(buf, BUFSIZ, "grid(0x%08x 0x%08x %d :request 0x%08x)%c", grid->id(), 
	       grid->selector(), LinkSelection::WaitingToBeSelected, sessionid(), '\0');
      SendCmdString(link, buf);
    }
  }
}
  
// handle reserve request from remote DrawLink.
void DrawServ::grid_message_handle(DrawLink* link, unsigned int id, unsigned int selector, 
				   int state, unsigned int newselector)
{
  void* ptr = nil;
  gridtable()->find(ptr, id);
  if (ptr) {
    GraphicId* grid = (GraphicId*)ptr;
    if (selector==sessionid() && newselector!=0) {
      if (grid->selector()==sessionid() && grid->selected()==LinkSelection::NotSelected) {
	grid->selector(newselector);
	char buf[BUFSIZ];
	snprintf(buf, BUFSIZ, "grid(0x%08x 0x%08x :grant 0x%08x)%c",
		 grid->id(), newselector, sessionid(), '\0');
	SendCmdString(link, buf);
      }
    } else {
      grid->selector(selector);
      grid->selected(state);
    }
  }
}

// handle callback from remote DrawLink.
void DrawServ::grid_message_callback(DrawLink* link, unsigned int id, unsigned int selector, 
				     int state, unsigned int oldselector)
{
  void* ptr = nil;
  gridtable()->find(ptr, id);
  if (ptr) {
    GraphicId* grid = (GraphicId*)ptr;
    grid->selector(selector);
  }
}

// handle reserve request from remote DrawLink.
void DrawServ::grid_message_propagate(DrawLink* link, unsigned int id, unsigned int selector, 
				      int selected, unsigned int otherselector)
{
  void* ptr = nil;
  gridtable()->find(ptr, id);
  if (ptr) {
    GraphicId* grid = (GraphicId*)ptr;
  }
}

void DrawServ::print_gridtable() {
  GraphicIdTable* table = gridtable();
  GraphicIdTable_Iterator it(*table);
  printf("id          &grid       &grcomp     selector    selected\n");
  printf("----------  ----------  ----------  ----------  --------\n");
  while(it.more()) {
    GraphicId* grid = (GraphicId*)it.cur_value();
    printf("0x%08x  0x%08x  0x%08x  0x%08x  %s\n", 
	   (unsigned int)it.cur_key(), grid, grid->grcomp(),
	   grid->selector(), LinkSelection::selected_string(grid->selected()));
    it.next();
  }
}

void DrawServ::print_sidtable() {
  printf("not implemented yet\n");
}
