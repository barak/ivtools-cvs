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
  reset_stack();

#if __GNUC__==3&&__GNUC_MINOR__<1
  fprintf(stderr, "Please upgrade to gcc-3.1 or greater\n");
  push_stack(ComValue::nullval());
  return;
#endif

  if (hostv.is_string() && portv.is_known() && statev.is_known()) {
    
    const char* hoststr = hostv.string_ptr();
    const char* portstr = portv.is_string() ? portv.string_ptr() : nil;
    u_short portnum = portstr ? atoi(portstr) : portv.ushort_val();
    u_short statenum = statev.ushort_val();
    int lidnum = lidv.is_known() ? lidv.int_val() : -1;
    int ridnum = ridv.is_known() ? ridv.int_val() : -1;

    DrawLink* link = 
      ((DrawServ*)unidraw)->linkup(hoststr, portnum, statenum, 
				   lidnum, ridnum, this->comterp());
    if (link) {
      DrawLinkComp* linkcomp = new DrawLinkComp(link);
      ComValue result(DrawLinkComp::class_symid(), new ComponentView(linkcomp));
      result.object_compview(true);
      push_stack(result);
    }
    else
      push_stack(ComValue::nullval());
  } 
    
  return;

#endif

}
