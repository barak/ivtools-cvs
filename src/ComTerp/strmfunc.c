/*
 * Copyright (c) 2001 Scott E. Johnston
 * Copyright (c) 1994-1997 Vectaport Inc.
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

#include <ComTerp/strmfunc.h>
#include <ComTerp/comvalue.h>
#include <ComTerp/comterp.h>
#include <Attribute/attrlist.h>
#include <Attribute/attribute.h>
#include <Unidraw/iterator.h>

#define TITLE "StrmFunc"

#define STREAM_MECH

/*****************************************************************************/

StrmFunc::StrmFunc(ComTerp* comterp) : ComFunc(comterp) {
}

/*****************************************************************************/

int StreamFunc::_symid;

StreamFunc::StreamFunc(ComTerp* comterp) : StrmFunc(comterp) {
}

void StreamFunc::execute() {
  ComValue operand1(stack_arg(0));
  
  reset_stack();
  
  if (operand1.is_stream()) {
    
    /* invoked by the next command */
    AttributeValueList* avl = operand1.stream_list();
    if (avl) {
      Iterator i;
      avl->First(i);
      AttributeValue* retval = avl->Done(i) ? nil : avl->GetAttrVal(i);
      if (retval) {
	push_stack(*retval);
	avl->Remove(retval);
	delete retval;
      } else {
	operand1.stream_list(nil);
	push_stack(ComValue::nullval());
      }
    } else
      push_stack(ComValue::nullval());
    
  } else {
    
    /* conversion operator */
    if (operand1.is_array()) {
      AttributeValueList* avl = new AttributeValueList(operand1.array_val());
      ComValue stream(this, avl);
      stream.stream_mode(-1); // for internal use (use by this func)
      push_stack(stream);
    } else if (operand1.is_attributelist()) {
      AttributeValueList* avl = new AttributeValueList();
      AttributeList* al = (AttributeList*)operand1.obj_val();
      Iterator i;
      for(al->First(i); !al->Done(i); al->Next(i)) {
	Attribute* attr = al->GetAttr(i);
	AttributeValue* av = 
	  new AttributeValue(Attribute::class_symid(), (void*)attr);
	avl->Append(av);
      }
      ComValue stream(this, avl);
      stream.stream_mode(-1); // for internal use (use by this func)
      push_stack(stream);
    }
    
  }
}

/*****************************************************************************/

int ConcatFunc::_symid;

ConcatFunc::ConcatFunc(ComTerp* comterp) : StrmFunc(comterp) {
}

void ConcatFunc::execute() {
  ComValue operand1(stack_arg_post_eval(0));
  ComValue operand2(stack_arg_post_eval(1));
  reset_stack();

  /* setup for concatenation */
  static ConcatNextFunc* cnfunc = new ConcatNextFunc(comterp());
  AttributeValueList* avl = new AttributeValueList();
  avl->Append(new AttributeValue(operand1));
  avl->Append(new AttributeValue(operand2));
  ComValue stream(cnfunc, avl);
  stream.stream_mode(-1); // for internal use (use by ConcatNextFunc)
  push_stack(stream);
}

/*****************************************************************************/

int ConcatNextFunc::_symid;

ConcatNextFunc::ConcatNextFunc(ComTerp* comterp) : StrmFunc(comterp) {
}

void ConcatNextFunc::execute() {
  ComValue operand1(stack_arg(0));

  /* invoked by next func */
  reset_stack();
  AttributeValueList* avl = operand1.stream_list();
  if (avl) {
    Iterator i;
    avl->First(i);
    AttributeValue* oneval = avl->GetAttrVal(i);
    avl->Next(i);
    AttributeValue* twoval = avl->GetAttrVal(i);
    boolean done = false;
    
    /* stream first argument until nil */
    if (oneval->is_known()) {
      if (oneval->is_stream()) {
	NextFunc nextfunc(comterp());
	push_stack(*oneval);
	push_funcstate(1,0);
	nextfunc.execute();
	pop_funcstate();
	if (comterp()->stack_top().is_unknown()) {
	  *oneval = ComValue::nullval();
	  comterp()->pop_stack();
	} else
	  done = true;
      } else {
	push_stack(*oneval);
	*oneval = ComValue::nullval();
	done = true;
      }
    }
    
    /* stream 2nd argument until nil */
    if (twoval->is_known() && !done) {
      if (twoval->is_stream()) {
	NextFunc nextfunc(comterp());
	push_stack(*twoval);
	push_funcstate(1,0);
	nextfunc.execute();
	pop_funcstate();
	if (comterp()->stack_top().is_unknown())
	  *twoval = ComValue::nullval();
      } else {
	push_stack(*twoval);
	*twoval = ComValue::nullval();
      }
    } else if (!done) 
      push_stack(ComValue::nullval());
    
  } else
    push_stack(ComValue::nullval());

  return;
}

/*****************************************************************************/

RepeatFunc::RepeatFunc(ComTerp* comterp) : StrmFunc(comterp) {
}

void RepeatFunc::execute() {
    ComValue operand1(stack_arg(0));

#ifdef STREAM_MECH
    if (operand1.is_stream()) {
      reset_stack();
      AttributeValueList* avl = operand1.stream_list();
      if (avl) {
	Iterator i;
	avl->First(i);
	AttributeValue* repval = avl->GetAttrVal(i);
	avl->Next(i);
	AttributeValue* cntval = avl->GetAttrVal(i);
	if (cntval->int_val()>0)
	  push_stack(*repval);
	else
	  push_stack(ComValue::nullval());
	cntval->int_ref()--;
      } else
	push_stack(ComValue::nullval());
      return;
    }
#endif

    ComValue operand2(stack_arg(1));
    reset_stack();

    if (operand1.is_nil() || operand2.is_nil()) {
      push_stack(ComValue::nullval());
      return;
    }

    int n = operand2.int_val();
    if (n<=0) return;

#ifdef STREAM_MECH
    AttributeValueList* avl = new AttributeValueList();
    avl->Append(new AttributeValue(operand1));
    avl->Append(new AttributeValue(operand2));
    ComValue stream(this, avl);
    stream.stream_mode(-1); // for internal use (use by this func)
    push_stack(stream);
#else
    AttributeValueList* avl = new AttributeValueList();
    for (int i=0; i<n; i++) 
        avl->Append(new ComValue(operand1));
    ComValue array(avl);
    push_stack(array);
#endif
}

/*****************************************************************************/

IterateFunc::IterateFunc(ComTerp* comterp) : StrmFunc(comterp) {
}

void IterateFunc::execute() {
    ComValue operand1(stack_arg(0));

#ifdef STREAM_MECH
    if (operand1.is_stream()) {
      reset_stack();
      AttributeValueList* avl = operand1.stream_list();
      if (avl) {
	Iterator i;
	avl->First(i);
	AttributeValue* startval = avl->GetAttrVal(i);
	avl->Next(i);
	AttributeValue* stopval = avl->GetAttrVal(i);
	avl->Next(i);
	AttributeValue* nextval = avl->GetAttrVal(i);
	push_stack(*nextval);
	if (nextval->int_val()==stopval->int_val()) 
	  *nextval = ComValue::nullval();
	else {
	  if (startval->int_val()<=stopval->int_val())
	    nextval->int_ref()++;
	  else
	    nextval->int_ref()--;
	}
      } else
	push_stack(ComValue::nullval());
      return;
    }
#endif

    ComValue operand2(stack_arg(1));
    reset_stack();

    if (operand1.is_nil() || operand2.is_nil()) {
      push_stack(ComValue::nullval());
      return;
    }

    int start = operand1.int_val();
    int stop = operand2.int_val();
#ifdef STREAM_MECH
    AttributeValueList* avl = new AttributeValueList();
    avl->Append(new AttributeValue(operand1));
    avl->Append(new AttributeValue(operand2));
    avl->Append(new AttributeValue(operand1));
    ComValue stream(this, avl);
    stream.stream_mode(-1); // for internal use (use by this func)
    push_stack(stream);
#else
    int dir = start>stop ? -1 : 1;

    AttributeValueList* avl = new AttributeValueList();
    for (int i=start; i!=stop; i+=dir) 
        avl->Append(new ComValue(i, ComValue::IntType));
    avl->Append(new ComValue(stop, ComValue::IntType));
    ComValue array(avl);
    push_stack(array);
#endif
}

/*****************************************************************************/

NextFunc::NextFunc(ComTerp* comterp) : StrmFunc(comterp) {
}

void NextFunc::execute() {
    ComValue streamv(stack_arg(0));
    reset_stack();

    if (!streamv.is_stream()) return;

    if (streamv.stream_mode()<0) {
      push_stack(streamv);
      push_funcstate(1, 0);
      ((ComFunc*)streamv.stream_func())->execute();
      pop_funcstate();
    } else if (streamv.stream_mode()>0) {
      ComFunc* funcptr = (ComFunc*)streamv.stream_func();
      AttributeValueList* avl = streamv.stream_list();
      int narg=0;
      int nkey=0;
      if (funcptr && avl) {
	Iterator i;
	avl->First(i);
	while (!avl->Done(i)) {
	  AttributeValue* val =  avl->GetAttrVal(i);

	  if (val->is_stream()) {
	    push_stack(*val);
	    push_funcstate(1,0);
	    if (val->stream_mode()<0 && val->stream_func()) {
	      /* internal use */
	      ((ComFunc*)val->stream_func())->execute();
	      if (comterp()->stack_top().is_null()) val->stream_list(nil);
	    }
	    else {
	      /* external use */
	      this->execute();  
	    }
	    pop_funcstate();
	    narg++;

	  } else {
	    push_stack(*val);
	    if (val->is_key()) 
	      nkey++;
	    else
	      narg++;
	  }
	  avl->Next(i);
	}
	push_funcstate(narg, nkey);
	funcptr->execute();
	pop_funcstate();
	if (comterp()->stack_top().is_null()) streamv.stream_list(nil);
      }
    }
}

