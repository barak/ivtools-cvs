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

//: Unidraw specialized for DrawServ
// Unidraw (OverlayUnidraw) specialized for DrawServ application.
// Networked application of the Unidraw framework.

class AttributeValueList;
class ComTerp;
class DrawLink;
class DrawLinkList;
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
    // execute command string locally, and on remote DrawServ's

    DrawLinkList* linklist() { return _linklist; }
    // return pointer to list of DrawLink's


    /* mechanism for reserving graphic ids */

    void reserve_batch_request(int nids);
    // initiate reservation of a set of graphic ids.
    void reserve_batch_response(int reserved, unsigned int batchid);
    // finalize reservation of a set of graphic ids
    void reserve_batch_handle(DrawLink* link, unsigned int* ids, int nids, unsigned int batchid);
    // respond to a request to reserve ids from a particular DrawLink
    int reserved_id();
    // next available reserved id.
    int reserved_batch();
    // process next available batch of reserved id's.

    static int reserve_batch_min() { return _reserve_batch_min; }
    // get the minimum number of reserved id batches allowed.
    static void reserve_batch_min(int min) { _reserve_batch_min = min; }
    // set the minimum number of reserved id batches allowed.
    static int reserve_batch_size() { return _reserve_batch_size; }
    // get the batch size for a request to reserve ids
    static void reserve_batch_size(int size) { _reserve_batch_size = size; }
    // set the batch size for a request to reserve ids

    GraphicIdTable* gridtable() { return _gridtable; }
    // return pointer to table of GraphicId's.

protected:
    DrawLinkList* _linklist;
    // DrawLink list
    GraphicIdList* _grid_request_list;
    // GraphicId reservation request list
    GraphicIdList* _grid_reserved_list;
    // GraphicId reservation completed list
    GraphicIdTable* _gridtable;
    // table of all GraphicId's.

    static int _reserve_batch_min;
    static int _reserve_batch_size;
};

#endif
