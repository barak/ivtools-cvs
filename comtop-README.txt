I. This directory contains the starter files necessary for creating
custom comterp servers:

	Imakefile	file to convert to Makefile using "ivmkmf -a"

	main.cc		simplified version of comterp's main.c, 
			hardwired for server mode

	newfunc.h	sample of a class derived from ComFunc	 
	newfunc.cc

	newhandler.h    sample of a class derived from ComHandler
	newhandler.cc

II. To build the sample program, set your CPU environment
variable the same way you did for building ivtools (see
http://www.vectaport.com/ivtools/ivtools-0.7-INSTALL.txt), then type:

	ivmkmf -a
	make

The resultant program is in $CPU/a.out.

III. To extend a comterp server with new functions (derived ComFunc's)
is a two step process:

	1) derive a new ComFunc -- NewFunc in newfunc.cc is an example
	of this.

	2) derive a new ComHandler that adds new the function to the
	comterp server -- NewHandler in newhandler.cc (and its
	associated template class, NewAcceptor) is an example of this.


IV.  To add new arguments to a derived ComFunc is also a two step
process (only the first is required to make it work):

	1) add a new call to stack_arg or stack_key at the front of
	the ::execute method, before the call to reset stack.

	2) add a description of the new argument to the ::docstring
	method.

For further information on comterp programming see:

http://www.vectaport.com/ivtools/comterp.html

Vectaport Inc.
Feb 1999


