/*
 * Copyright (c) 1997,1999 Vectaport Inc.
 * Copyright (c) 1994-1996 Vectaport Inc., Cartoactive Systems
 * Copyright (c) 1993 David B. Hollenbeck
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
 * RemoteConnectDialog related classes
 */

#include <DrawServ/drawserv.h>
#include <DrawServ/rcdialog.h>

#include <Unidraw/editor.h>

#include <IVGlyph/stredit.h>

#include <InterViews/window.h>

#include <stdio.h>

/*****************************************************************************/

RemoteConnectAction::RemoteConnectAction(Editor* editor) : Action() {
  _editor = editor;
}

void RemoteConnectAction::execute() {
  char* hostname =
    StrEditDialog::post
    (_editor->GetWindow(), 
     "Enter host name to connect:                  ", "localhost");
  if (hostname) ((DrawServ*)unidraw)->linkup(hostname, 20002, 0);
}

