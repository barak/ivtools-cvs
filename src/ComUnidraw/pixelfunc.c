/*
 * Copyright (c) 2001 Scott E. Johnston
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

#include <ComUnidraw/pixelfunc.h>

#include <OverlayUnidraw/ovraster.h>
#include <Unidraw/Graphic/damage.h>
#include <Unidraw/viewer.h>

/*****************************************************************************/

PixelPokeFunc::PixelPokeFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void PixelPokeFunc::execute() {
  Viewer* viewer = _ed->GetViewer();

  ComValue rastcompv(stack_arg(0));
  ComValue xv(stack_arg(1));
  ComValue yv(stack_arg(2));
  ComValue valv(stack_arg(3));
  reset_stack();
  
  RasterOvComp* rastcomp = (RasterOvComp*) rastcompv.geta(RasterOvComp::class_symid());
  OverlayRasterRect* rastrect = rastcomp ? rastcomp->GetOverlayRasterRect() : nil;
  OverlayRaster* raster = rastrect ? rastrect->GetOriginal() : nil;

  if (raster) {
    raster->poke(xv.int_val(), yv.int_val(), valv.float_val(), valv.float_val(), valv.float_val(), 1.0);
    push_stack(rastcompv);
  } else 
    push_stack(ComValue::nullval());


}

/*****************************************************************************/

PixelColsFunc::PixelColsFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void PixelColsFunc::execute() {
  Viewer* viewer = _ed->GetViewer();

  ComValue rastcompv(stack_arg(0));
  reset_stack();
  
  RasterOvComp* rastcomp = (RasterOvComp*) rastcompv.geta(RasterOvComp::class_symid());
  OverlayRasterRect* rastrect = rastcomp ? rastcomp->GetOverlayRasterRect() : nil;
  OverlayRaster* raster = rastrect ? rastrect->GetOriginal() : nil;

  if (raster) {
    ComValue retval(raster->pwidth());
    push_stack(retval);
  } else 
    push_stack(ComValue::nullval());


}

/*****************************************************************************/

PixelRowsFunc::PixelRowsFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void PixelRowsFunc::execute() {
  Viewer* viewer = _ed->GetViewer();

  ComValue rastcompv(stack_arg(0));
  reset_stack();
  
  RasterOvComp* rastcomp = (RasterOvComp*) rastcompv.geta(RasterOvComp::class_symid());
  OverlayRasterRect* rastrect = rastcomp ? rastcomp->GetOverlayRasterRect() : nil;
  OverlayRaster* raster = rastrect ? rastrect->GetOriginal() : nil;

  if (raster) {
    ComValue retval(raster->pheight());
    push_stack(retval);
  } else 
    push_stack(ComValue::nullval());


}

/*****************************************************************************/

PixelFlushFunc::PixelFlushFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void PixelFlushFunc::execute() {
  Viewer* viewer = _ed->GetViewer();

  ComValue rastcompv(stack_arg(0));
  reset_stack();
  
  RasterOvComp* rastcomp = (RasterOvComp*) rastcompv.geta(RasterOvComp::class_symid());
  OverlayRasterRect* rastrect = rastcomp ? rastcomp->GetOverlayRasterRect() : nil;
  OverlayRaster* raster = rastrect ? rastrect->GetOriginal() : nil;

  if (raster) {
    raster->flush();
    viewer->GetDamage()->Incur(rastrect);
    ComValue retval(rastcompv);
    push_stack(retval);
  } else 
    push_stack(ComValue::nullval());


}

