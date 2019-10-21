/*
 * include/x86.h
 *
 * This work was supported by the Director, Office of Science, Division
 * of Mathematical, Information, and Computational Sciences of the
 * U.S. Department of Energy under contract number DE-AC03-76SF00098.
 *
 * Copyright (c) 2001
 *
 * Contains functions to set and restore the round-to-double flag in the
 * control word of a x86 FPU.  The algorithms in the double-double and
 * quad-double package does not function with the extended mode found in
 * these FPU.
 */
#ifndef _X86_H_
#define _X86_H_

#define x86

#define _NO_CHANGE      0
#define _ADD_UNDERSCORE 1
#define _UPPER_CASE     2

#ifdef ADD_UNDERSCORE
#define NAMING _ADD_UNDERSCORE
#endif

#ifdef UPPER_CASE
#define NAMING _UPPER_CASE
#endif

#ifndef NAMING
#define NAMING _NO_CHANGE
#endif

#if (NAMING == _ADD_UNDERSCORE)
#define x86_fix_start x86_fix_start_
#define x86_fix_end   x86_fix_end_
#endif

#if (NAMING == _UPPER_CASE)
#define x86_fix_start X86_FIX_START
#define x86_fix_end   X86_FIX_END
#endif

extern "C" {

/*
 * Set the round-to-double flag, and save the old control word in old_cw.
 * If old_cw is NULL, the old control word is not saved.
 */
void x86_fix_start(unsigned short *old_cw);

/*
 * Restore the control word.
 */
void x86_fix_end(unsigned short *old_cw);

}

#endif  /* _X86_H_ */
