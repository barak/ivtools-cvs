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

//: Unidraw specialized for DrawServ
// Unidraw (OverlayUnidraw) specialized for DrawServ application.
// Networked application of the Unidraw framework.

class DrawLinkList;

class DrawServ : public OverlayUnidraw {
public:
    DrawServ(
        Catalog*, int& argc, char** argv, 
        OptionDesc* = nil, PropertyData* = nil
    );
    DrawServ(Catalog*, World*);
    virtual ~DrawServ();

    void Init();

    int linkup(const char* hostname, int portnum, 
	       int state, int local_id=-1, int remote_id=-1);
    // Create new link to remote drawserv, return -1 if error
    // state: 0==new_link, 1==one_way, 2==two_way.
    // Let DrawLink assign local_id by passing -1 for local_id.
    // The local_id argument is for verification purposes once
    // two-way link is established.

protected:
    DrawLinkList* _list;
};

#endif
