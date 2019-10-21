/*
 * src/x86.cc
 *
 * This work was supported by the Director, Office of Science, Division
 * of Mathematical, Information, and Computational Sciences of the
 * U.S. Department of Energy under contract number DE-AC03-76SF00098.
 *
 * Copyright (c) 2000-2001
 *
 * Contains functions to set and restore the round-to-double flag in the
 * control word of a x86 FPU.
 */


#include "x86.h"

/*
#ifdef X86
#ifdef  _WIN32
#include <float.h>
#else

#ifdef HAVE_FPU_CONTROL_H
#include <fpu_control.h>
#endif

#ifndef _FPU_GETCW
#define _FPU_GETCW(x) asm volatile ("fnstcw %0":"=m" (x));
#endif

#ifndef _FPU_SETCW
#define _FPU_SETCW(x) asm volatile ("fldcw %0": :"m" (x));
#endif

#ifndef _FPU_EXTENDED
#define _FPU_EXTENDED 0x0300
#endif

#ifndef _FPU_DOUBLE
#define _FPU_DOUBLE 0x0200
#endif

#endif
#endif // X86

extern "C" {

void x86_fix_start(unsigned short *old_cw) {
#ifdef X86
#ifdef _WIN32
  // Win 32 MSVC
  unsigned int cw = _control87(0, 0);
  _control87(0x00010000, 0x00030000);
  if (old_cw) {
    *old_cw = cw;
  }
#else
  // Linux
  int cw, new_cw;
  _FPU_GETCW(cw);

  new_cw = (cw & ~_FPU_EXTENDED) | _FPU_DOUBLE;
  _FPU_SETCW(new_cw);

  if (old_cw) {
    *old_cw = cw;
  }
#endif
#endif
}

void x86_fix_end(unsigned short *old_cw) {
#ifdef X86
#ifdef _WIN32
  // Win 32 MSVC
  if (old_cw) {
    _control87(*old_cw, 0xFFFFFFFF);
  }
#else
  // Linux
  if (old_cw) {
    int cw;
    cw = *old_cw;
    _FPU_SETCW(cw);
  }
#endif
#endif
}

}
*/

#include <cstdlib>

#include "x86.h"

#ifdef x86
#ifndef _FPU_GETCW
#ifndef __GNUG__
inline void _FPU_GETCW(unsigned short & x) { __asm fnstcw DWORD PTR [x] }
#else
#define _FPU_GETCW(x) asm volatile ("fnstcw %0":"=m" (x));
#endif
#endif

#ifndef _FPU_SETCW
#ifndef __GNUG__
inline void _FPU_SETCW(unsigned short & x) { __asm fldcw DWORD PTR [x] }
#else
#define _FPU_SETCW(x) asm volatile ("fldcw %0": :"m" (x));
#endif
#endif

#ifndef _FPU_EXTENDED
#define _FPU_EXTENDED 0x0300
#endif

#ifndef _FPU_DOUBLE
#define _FPU_DOUBLE 0x0200
#endif
#endif  // x86

extern "C" {

void x86_fix_start(unsigned short *old_cw) {
#ifdef x86
  unsigned short new_cw;
  unsigned short dummy;

  if (old_cw == NULL)
    old_cw = &dummy;

  _FPU_GETCW(*old_cw);
  new_cw = (*old_cw & ~_FPU_EXTENDED) | _FPU_DOUBLE;
  _FPU_SETCW(new_cw);
#endif
}

void x86_fix_end(unsigned short *old_cw) {
#ifdef x86
  if (old_cw != NULL) {
    _FPU_SETCW(*old_cw);
  }
#endif
}

}

