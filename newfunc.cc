/*
 * Copyright (c) 1998 Vectaport Inc.
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

#include "newfunc.h"
#include <ComTerp/comvalue.h>
#include <ComTerp/comterpserv.h>

NewFunc::NewFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void NewFunc::execute() {


  /* ----- ARGUMENT PROCESSING ----- */

  // get id for keyword symbol(s) from symbol table for ::stack_key()
  // static declaraction makes it happen only once
  static int key1_symid = symbol_add("key1"); 

  // retrieve fixed arguments from the interpreter stack
  ComValue string1(stack_arg(0));
  ComValue string2(stack_arg(1));

  // retrieve keyword arguments from the interpreter stack
  // will default to 1 if :key1 is provided without subsequent value
  ComValue key1(stack_key(key1_symid));

  // reset the stack to remove all the operands associated with this func
  // this deconstructs the ComValue's on the stack
  reset_stack();

	
  /* ----- FUNCTION BODY ----- */

  // use ComValue access methods and type testing methods
  // (defined in Attribute/attrvalue.h) to retrieve type-specific data
  if (string1.is_string()) 
    cerr << "the first argument:  " << string1.string_ptr() << "\n";
  else
    cerr << "the first argument is not a string:  " << string1 << "\n";
  if (string2.is_string()) 
    cerr << "the second argument:  " << string2.string_ptr() << "\n";
  else
    cerr << "the second argument is not a string:  " << string2 << "\n";
  cerr << "the first keyword:  " << key1 << "\n";


  /* ----- RETURN VALUE ----- */

  // create a ComValue initialized with the return value, 
  // and push it on the stack	
  ComValue retval(string2);
  push_stack(retval);


  return;
}
