/*
 * Copyright (c) 2004 Scott E. Johnston
 * Copyright (c) 1994-1997,1999 Vectaport Inc.
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
 * RemoteConnectDialog - classes related to the remote connect dialog box
 */

#ifndef rcdialog_h
#define rcdialog_h

#include <InterViews/action.h>
#include <Unidraw/editor.h>

class Editor;
class RemoteConnectDialog;

//: action to establish a connection with a remote DrawServ
class RemoteConnectAction : public Action {
public:
  RemoteConnectAction(Editor*);
  virtual void execute();
protected:
  Editor* _editor;
};

#include <InterViews/_leave.h>

#endif
