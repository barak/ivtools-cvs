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

#include <DrawServ/draweditor.h>
#include <DrawServ/drawfunc.h>
#include <DrawServ/drawlink.h>
#include <DrawServ/drawlinkcomp.h>
#include <DrawServ/drawserv.h>

#include <Attribute/attrlist.h>
#include <Attribute/attrvalue.h>

#define TITLE "DrawLinkFunc"

/*****************************************************************************/

DrawLinkFunc::DrawLinkFunc(ComTerp* comterp, DrawEditor* ed) : UnidrawFunc(comterp, ed) {
}

void DrawLinkFunc::execute() {
#ifndef HAVE_ACE

  reset_stack();
  fprintf(stderr, "rebuild ivtools with ACE support to get full drawserv functionality\n");
  push_stack(ComValue::nullval());

#else

  ComValue hostv(stack_arg(0, true));
  static int port_sym = symbol_add("port");
  ComValue default_port(20002);
  ComValue portv(stack_key(port_sym, false, default_port, true));
  static int state_sym = symbol_add("state");
  ComValue default_state(0);
  ComValue statev(stack_key(state_sym, false, default_state, true));
  static int lid_sym = symbol_add("lid");
  ComValue lidv(stack_key(lid_sym));
  static int rid_sym = symbol_add("rid");
  ComValue ridv(stack_key(rid_sym));
  static int close_sym = symbol_add("close");
  ComValue closev(stack_key(close_sym));
  reset_stack();

#if __GNUC__==3&&__GNUC_MINOR__<1
  fprintf(stderr, "Please upgrade to gcc-3.1 or greater\n");
  push_stack(ComValue::nullval());
  return;
#endif

  DrawLink* link = nil;

  /* creating a new link to remote drawserv */
  if (hostv.is_string() && portv.is_known() && statev.is_known()) {
    
    const char* hoststr = hostv.string_ptr();
    const char* portstr = portv.is_string() ? portv.string_ptr() : nil;
    u_short portnum = portstr ? atoi(portstr) : portv.ushort_val();
    u_short statenum = statev.ushort_val();
    int lidnum = lidv.is_known() ? lidv.int_val() : -1;
    int ridnum = ridv.is_known() ? ridv.int_val() : -1;

    link = 
      ((DrawServ*)unidraw)->linkup(hoststr, portnum, statenum, 
				   lidnum, ridnum, this->comterp());
  
     
  } 

  /* return pointer to existing link */
  else if (ridv.is_known() || lidv.is_known()) {
    link = ((DrawServ*)unidraw)->linkget
      (lidv.is_known() ? lidv.int_val() : -1, 
       ridv.is_known() ? ridv.int_val() : -1);

    /* close if that flag is set. */
    if (link && closev.is_true()) {
      ((DrawServ*)unidraw)->linkdown(link);
      link = nil;
    }
    
  }

  /* dump DrawLink table to stderr */
  else 
    ((DrawServ*)unidraw)->linkdump(stderr);
  
  if (link) {
    DrawLinkComp* linkcomp = new DrawLinkComp(link);
    ComValue result(DrawLinkComp::class_symid(), new ComponentView(linkcomp));
    result.object_compview(true);
    push_stack(result);
  }
  else
    push_stack(ComValue::nullval());

  return;

#endif

}


/*****************************************************************************/

SessionIdFunc::SessionIdFunc(ComTerp* comterp, DrawEditor* ed) : UnidrawFunc(comterp, ed) {
}

void SessionIdFunc::execute() {
#ifndef HAVE_ACE

  reset_stack();
  fprintf(stderr, "rebuild ivtools with ACE support to get full drawserv functionality\n");
  push_stack(ComValue::nullval());

#else
  static int new_sym = symbol_add("new");
  ComValue newv(stack_key(new_sym));

  static int trial_sym = symbol_add("trial");
  ComValue trialv(stack_key(trial_sym));

  static int rid_sym = symbol_add("rid");
  ComValue ridv(stack_key(rid_sym));

  static int ok_sym = symbol_add("ok");
  ComValue okv(stack_key(ok_sym));

  ComValue idv(stack_arg(0));

  reset_stack();

#if __GNUC__==3&&__GNUC_MINOR__<1
  fprintf(stderr, "Please upgrade to gcc-3.1 or greater\n");
  push_stack(ComValue::nullval());
  return;
#endif

  unsigned int returnid = 0;

  /* request a new session id */
  if (newv.is_true()) {    
    ((DrawServ*)unidraw)->sessionid_request_new();
    returnid = ((DrawServ*)unidraw)->sessionid(true);
  }

  /* return the trial id */
  else if (trialv.is_true()) {    
    returnid = ((DrawServ*)unidraw)->sessionid(true);
  }

  /* handle new session id request */
  else if (idv.is_known() && ridv.is_known()) {
    returnid = idv.uint_val();
    ((DrawServ*)unidraw)->sessionid_handle_new(returnid, ridv.int_val());
  }

  /* handle callback */ 
  else if (okv.is_known() && ridv.is_known()) {
    returnid = idv.uint_val();
    ((DrawServ*)unidraw)->sessionid_callback_new(ridv.int_val(), returnid, okv.is_true());
  }

  /* return current session id */
  else if (nargs()==0 && nkeys()==0) 
    returnid = ((DrawServ*)unidraw)->sessionid();

  ComValue retval(returnid);
  push_stack(retval);
  return;

#endif

}


/*****************************************************************************/

ReserveFunc::ReserveFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void ReserveFunc::execute() {
#if 0
#if 1
  static int batchid_symid = symbol_add("batchid");
  ComValue batchidv(stack_key(batchid_symid));
  unsigned int batchid = batchidv.is_known() ? batchidv.uint_val() : 0;

  static int batch_symid = symbol_add("batch");
  ComValue batch_flagv(stack_key(batch_symid));
  boolean batch_flag = batch_flagv.is_true();

  ComValue arrayv(stack_arg(0));

  reset_stack();

  /* provoke a batch request */
  if (batch_flag) 
    ((DrawServ*)unidraw)->reserve_batch_request(DrawServ::reserve_batch_size());

  /* handle a batch request */
  else if (arrayv.is_array()) {
    AttributeValueList* avl = arrayv.array_val();
    Iterator it;
    avl->First(it);
    const int nids = avl->Number();
    unsigned int ids[nids];
    for (int i=0; i<nids; i++) {
      ids[i] = avl->GetAttrVal(it)->uint_val();
      avl->Next(it);
    }
    ((DrawServ*)unidraw)->reserve_batch_handle(nil, ids, nids, batchid);
  }
    

#else
    static int all_symid = symbol_add("all");
    ComValue all_flagv(stack_key(all_symid));
    boolean all_flag = all_flagv.is_true();

    static int clear_symid = symbol_add("clear");
    ComValue clear_flagv(stack_key(clear_symid));
    boolean clear_flag = clear_flagv.is_true();

    Selection* sel = _ed->GetViewer()->GetSelection();
    if (clear_flag) {
      sel->Clear();
      reset_stack();
      return;
    }
      
    OverlaySelection* newSel = ((OverlayEditor*)_ed)->overlay_kit()->MakeSelection();
    
    Viewer* viewer = _ed->GetViewer();
    AttributeValueList* avl = new AttributeValueList();
    if (all_flag) {

      GraphicView* gv = ((OverlayEditor*)_ed)->GetFrame();
      Iterator i;
      int count=0;
      for (gv->First(i); !gv->Done(i); gv->Next(i)) {
	GraphicView* subgv = gv->GetView(i);
	newSel->Append(subgv);
	OverlayComp* comp = (OverlayComp*)subgv->GetGraphicComp();
	ComValue* compval = new ComValue(comp->classid(), new ComponentView(comp));
	compval->object_compview(true);
	avl->Append(compval);
      }

    } else if (nargs()==0) {
      Iterator i;
      int count=0;
      for (sel->First(i); !sel->Done(i); sel->Next(i)) {
	GraphicView* grview = sel->GetView(i);
	OverlayComp* comp = grview ? (OverlayComp*)grview->GetSubject() : nil;
	ComValue* compval = comp ? new ComValue(comp->classid(), new ComponentView(comp)) : nil;

	if (compval) {
	  compval->object_compview(true);
	  avl->Append(compval);
	}
	delete newSel;
        newSel = nil;
      }

    } else {

      for (int i=0; i<nargsfixed(); i++) {
        ComValue& obj = stack_arg(i);
	if (obj.object_compview()) {
	  ComponentView* comview = (ComponentView*)obj.obj_val();
	  OverlayComp* comp = (OverlayComp*)comview->GetSubject();
	  if (comp) {
	    newSel->Append(comp->FindView(viewer));
	    ComValue* compval = new ComValue(comp->classid(), new ComponentView(comp));
	    compval->object_compview(true);
	    avl->Append(compval);
	  }
	}
      }
    }

    if (newSel){
      sel->Clear();
      delete sel;
      _ed->SetSelection(newSel);
      newSel->Update(viewer);
      unidraw->Update();
    }
    reset_stack();
    ComValue retval(avl);
    push_stack(retval);
#endif
#endif
}

