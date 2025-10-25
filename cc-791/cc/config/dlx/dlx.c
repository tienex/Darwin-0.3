/* Subroutines for insn-output.c for DLX.
   Copyright (C) 1999 Apple Computer, Inc.

This file is part of GNU CC.

GNU CC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU CC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU CC; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#include "config.h"
#include <stdio.h>
#include "rtl.h"
#include "regs.h"
#include "hard-reg-set.h"
#include "real.h"
#include "insn-config.h"
#include "conditions.h"
#include "insn-flags.h"
#include "output.h"
#include "insn-attr.h"
#include "flags.h"
#include "tree.h"
#include "expr.h"
#include "function.h"
#include "recog.h"

/* Save the operands last given to a compare for use when we
   generate a scc or bcc insn.  */

rtx dlx_compare_op0, dlx_compare_op1;

/* Global variables for machine-dependent things.  */

/* Size of frame.  */
static int dlx_frame_size;

/* Number of registers saved.  */
static int dlx_saved_reg_count;

/* Compute and save the frame size and register mask.  */

static void
dlx_compute_frame_size(size)
     int size;
{
  int regno;
  int offset;

  dlx_frame_size = size;
  dlx_saved_reg_count = 0;

  /* Count callee-saved registers that need to be saved */
  for (regno = 9; regno <= 28; regno++)
    {
      if (regs_ever_live[regno] && !call_used_regs[regno])
	{
	  dlx_frame_size += 4;
	  dlx_saved_reg_count++;
	}
    }

  /* Save return address if this function calls others */
  if (regs_ever_live[31] || !leaf_function_p())
    {
      dlx_frame_size += 4;
      dlx_saved_reg_count++;
    }

  /* Round frame size to 8-byte boundary */
  dlx_frame_size = (dlx_frame_size + 7) & ~7;
}

/* Output function prologue.  */

void
dlx_function_prologue(file, size)
     FILE *file;
     int size;
{
  int regno;
  int offset;

  dlx_compute_frame_size(size);

  if (dlx_frame_size > 0)
    {
      fprintf(file, "\tsubi\tr29,r29,#%d\n", dlx_frame_size);
    }

  /* Save callee-saved registers */
  offset = dlx_frame_size;
  for (regno = 9; regno <= 28; regno++)
    {
      if (regs_ever_live[regno] && !call_used_regs[regno])
	{
	  offset -= 4;
	  fprintf(file, "\tsw\tr%d,%d(r29)\n", regno, offset);
	}
    }

  /* Save return address */
  if (regs_ever_live[31] || !leaf_function_p())
    {
      offset -= 4;
      fprintf(file, "\tsw\tr31,%d(r29)\n", offset);
    }

  /* Set up frame pointer if needed */
  if (frame_pointer_needed)
    {
      fprintf(file, "\taddi\tr30,r29,#%d\n", dlx_frame_size);
    }
}

/* Output function epilogue.  */

void
dlx_function_epilogue(file, size)
     FILE *file;
     int size;
{
  int regno;
  int offset;

  /* Restore callee-saved registers */
  offset = dlx_frame_size;
  for (regno = 9; regno <= 28; regno++)
    {
      if (regs_ever_live[regno] && !call_used_regs[regno])
	{
	  offset -= 4;
	  fprintf(file, "\tlw\tr%d,%d(r29)\n", regno, offset);
	}
    }

  /* Restore return address */
  if (regs_ever_live[31] || !leaf_function_p())
    {
      offset -= 4;
      fprintf(file, "\tlw\tr31,%d(r29)\n", offset);
    }

  /* Deallocate stack frame */
  if (dlx_frame_size > 0)
    {
      fprintf(file, "\taddi\tr29,r29,#%d\n", dlx_frame_size);
    }

  /* Return */
  fprintf(file, "\tjr\tr31\n");
}

/* Determine where to put an argument to a function.
   Value is zero to push the argument on the stack,
   or a hard register in which to store the argument.

   MODE is the argument's machine mode.
   TYPE is the data type of the argument (as a tree).
    This is null for libcalls where that information may
    not be available.
   CUM is a variable of type CUMULATIVE_ARGS which gives info about
    the preceding args and about the function being called.
   NAMED is nonzero if this argument is a named parameter
    (otherwise it is an extra parameter matching an ellipsis).

   On DLX the first 8 words of args are normally in registers r1-r8.  */

struct rtx_def *
dlx_function_arg(cum, mode, type, named)
     CUMULATIVE_ARGS *cum;
     enum machine_mode mode;
     tree type;
     int named;
{
  int regbase = 1;  /* First argument register */
  int regno;

  if (!named)
    return 0;

  /* Compute regno */
  regno = regbase + *cum;

  /* Check if we still have registers available */
  if (regno < 9)
    {
      /* Return in integer register for integers, float register for floats */
      if (GET_MODE_CLASS(mode) == MODE_FLOAT && TARGET_FPU)
	return gen_rtx(REG, mode, 32 + (regno - 1));
      else
	return gen_rtx(REG, mode, regno);
    }

  /* No more registers, pass on stack */
  return 0;
}

/* Print operand X (an rtx) in assembler syntax to file FILE.
   CODE is a letter or dot (`z' in `%z0').
   For `%' followed by punctuation, CODE is the punctuation and X is null.  */

void
dlx_print_operand(file, x, code)
     FILE *file;
     rtx x;
     int code;
{
  switch (code)
    {
    case 0:
      /* No special code */
      break;

    default:
      /* Unknown code */
      output_operand_lossage("invalid %%code");
      return;
    }

  if (GET_CODE(x) == REG)
    {
      fprintf(file, "r%d", REGNO(x));
    }
  else if (GET_CODE(x) == CONST_INT)
    {
      fprintf(file, "#%d", INTVAL(x));
    }
  else if (GET_CODE(x) == MEM)
    {
      dlx_print_operand_address(file, XEXP(x, 0));
    }
  else
    {
      output_addr_const(file, x);
    }
}

/* Print a memory address as an operand to reference that memory location.  */

void
dlx_print_operand_address(file, addr)
     FILE *file;
     rtx addr;
{
  switch (GET_CODE(addr))
    {
    case REG:
      fprintf(file, "0(r%d)", REGNO(addr));
      break;

    case PLUS:
      {
	rtx base = XEXP(addr, 0);
	rtx offset = XEXP(addr, 1);

	if (GET_CODE(base) == REG && GET_CODE(offset) == CONST_INT)
	  {
	    fprintf(file, "%d(r%d)", INTVAL(offset), REGNO(base));
	  }
	else if (GET_CODE(offset) == REG && GET_CODE(base) == CONST_INT)
	  {
	    fprintf(file, "%d(r%d)", INTVAL(base), REGNO(offset));
	  }
	else
	  {
	    output_addr_const(file, addr);
	  }
      }
      break;

    case SYMBOL_REF:
    case LABEL_REF:
    case CONST:
      output_addr_const(file, addr);
      break;

    default:
      output_addr_const(file, addr);
      break;
    }
}

/* Predicates for recognizing various operands.  */

/* Return 1 if OP is a register or the constant 0.  */

int
reg_or_0_operand(op, mode)
     rtx op;
     enum machine_mode mode;
{
  return (op == const0_rtx
	  || (GET_CODE(op) == CONST_INT && INTVAL(op) == 0)
	  || register_operand(op, mode));
}

/* Return 1 if OP is a register or a small integer that can be used
   as an immediate operand in most instructions (13-bit signed).  */

int
arith_operand(op, mode)
     rtx op;
     enum machine_mode mode;
{
  return (register_operand(op, mode)
	  || (GET_CODE(op) == CONST_INT
	      && INTVAL(op) >= -4096
	      && INTVAL(op) < 4096));
}

/* Return 1 if OP is a register or any integer that can be loaded
   with a single instruction.  */

int
arith32_operand(op, mode)
     rtx op;
     enum machine_mode mode;
{
  return (register_operand(op, mode)
	  || (GET_CODE(op) == CONST_INT));
}

/* Return 1 if OP is a small integer usable in an instruction.  */

int
small_int(op, mode)
     rtx op;
     enum machine_mode mode;
{
  return (GET_CODE(op) == CONST_INT
	  && INTVAL(op) >= -4096
	  && INTVAL(op) < 4096);
}

/* Return 1 if OP is a large integer that needs to be loaded
   with a multi-instruction sequence.  */

int
large_int(op, mode)
     rtx op;
     enum machine_mode mode;
{
  return (GET_CODE(op) == CONST_INT
	  && (INTVAL(op) < -4096 || INTVAL(op) >= 4096));
}

/* Return 1 if OP is a valid operand for a call instruction.  */

int
call_operand(op, mode)
     rtx op;
     enum machine_mode mode;
{
  return (GET_CODE(op) == MEM
	  && (GET_CODE(XEXP(op, 0)) == SYMBOL_REF
	      || GET_CODE(XEXP(op, 0)) == REG));
}
