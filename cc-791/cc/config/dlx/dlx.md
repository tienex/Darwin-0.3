;;- Machine description for DLX chip for GNU C compiler
;;  Copyright (C) 1999 Apple Computer, Inc.

;; This file is part of GNU CC.

;; GNU CC is free software; you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation; either version 2, or (at your option)
;; any later version.

;; GNU CC is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.

;; You should have received a copy of the GNU General Public License
;; along with GNU CC; see the file COPYING.  If not, write to
;; the Free Software Foundation, 59 Temple Place - Suite 330,
;; Boston, MA 02111-1307, USA.

;;- See file "rtl.def" for documentation on define_insn, match_*, et. al.

;; Insn type.  Used to default other attribute values.

(define_attr "type"
  "unknown,move,arith,compare,load,store,branch,jump,call,multi"
  (const_string "arith"))

;; Length (in # of insns).
(define_attr "length" ""
  (const_int 1))

;;
;; ....................
;;
;;  ADDITION
;;
;; ....................

(define_insn "addsi3"
  [(set (match_operand:SI 0 "register_operand" "=r,r")
	(plus:SI (match_operand:SI 1 "register_operand" "r,r")
		 (match_operand:SI 2 "arith_operand" "r,I")))]
  ""
  "@
   add\\t%0,%1,%2
   addi\\t%0,%1,%2"
  [(set_attr "type" "arith")])

;;
;; ....................
;;
;;  SUBTRACTION
;;
;; ....................

(define_insn "subsi3"
  [(set (match_operand:SI 0 "register_operand" "=r,r")
	(minus:SI (match_operand:SI 1 "register_operand" "r,r")
		  (match_operand:SI 2 "arith_operand" "r,I")))]
  ""
  "@
   sub\\t%0,%1,%2
   subi\\t%0,%1,%2"
  [(set_attr "type" "arith")])

;;
;; ....................
;;
;;  MULTIPLICATION
;;
;; ....................

(define_insn "mulsi3"
  [(set (match_operand:SI 0 "register_operand" "=r")
	(mult:SI (match_operand:SI 1 "register_operand" "r")
		 (match_operand:SI 2 "register_operand" "r")))]
  ""
  "mult\\t%0,%1,%2"
  [(set_attr "type" "arith")])

;;
;; ....................
;;
;;  DIVISION
;;
;; ....................

(define_insn "divsi3"
  [(set (match_operand:SI 0 "register_operand" "=r")
	(div:SI (match_operand:SI 1 "register_operand" "r")
		(match_operand:SI 2 "register_operand" "r")))]
  ""
  "div\\t%0,%1,%2"
  [(set_attr "type" "arith")])

(define_insn "udivsi3"
  [(set (match_operand:SI 0 "register_operand" "=r")
	(udiv:SI (match_operand:SI 1 "register_operand" "r")
		 (match_operand:SI 2 "register_operand" "r")))]
  ""
  "divu\\t%0,%1,%2"
  [(set_attr "type" "arith")])

;;
;; ....................
;;
;;  LOGICAL
;;
;; ....................

(define_insn "andsi3"
  [(set (match_operand:SI 0 "register_operand" "=r,r")
	(and:SI (match_operand:SI 1 "register_operand" "r,r")
		(match_operand:SI 2 "arith_operand" "r,I")))]
  ""
  "@
   and\\t%0,%1,%2
   andi\\t%0,%1,%2"
  [(set_attr "type" "arith")])

(define_insn "iorsi3"
  [(set (match_operand:SI 0 "register_operand" "=r,r")
	(ior:SI (match_operand:SI 1 "register_operand" "r,r")
		(match_operand:SI 2 "arith_operand" "r,I")))]
  ""
  "@
   or\\t%0,%1,%2
   ori\\t%0,%1,%2"
  [(set_attr "type" "arith")])

(define_insn "xorsi3"
  [(set (match_operand:SI 0 "register_operand" "=r,r")
	(xor:SI (match_operand:SI 1 "register_operand" "r,r")
		(match_operand:SI 2 "arith_operand" "r,I")))]
  ""
  "@
   xor\\t%0,%1,%2
   xori\\t%0,%1,%2"
  [(set_attr "type" "arith")])

(define_insn "one_cmplsi2"
  [(set (match_operand:SI 0 "register_operand" "=r")
	(not:SI (match_operand:SI 1 "register_operand" "r")))]
  ""
  "xori\\t%0,%1,#-1"
  [(set_attr "type" "arith")])

;;
;; ....................
;;
;;  SHIFTS
;;
;; ....................

(define_insn "ashlsi3"
  [(set (match_operand:SI 0 "register_operand" "=r,r")
	(ashift:SI (match_operand:SI 1 "register_operand" "r,r")
		   (match_operand:SI 2 "arith_operand" "r,I")))]
  ""
  "@
   sll\\t%0,%1,%2
   slli\\t%0,%1,%2"
  [(set_attr "type" "arith")])

(define_insn "ashrsi3"
  [(set (match_operand:SI 0 "register_operand" "=r,r")
	(ashiftrt:SI (match_operand:SI 1 "register_operand" "r,r")
		     (match_operand:SI 2 "arith_operand" "r,I")))]
  ""
  "@
   sra\\t%0,%1,%2
   srai\\t%0,%1,%2"
  [(set_attr "type" "arith")])

(define_insn "lshrsi3"
  [(set (match_operand:SI 0 "register_operand" "=r,r")
	(lshiftrt:SI (match_operand:SI 1 "register_operand" "r,r")
		     (match_operand:SI 2 "arith_operand" "r,I")))]
  ""
  "@
   srl\\t%0,%1,%2
   srli\\t%0,%1,%2"
  [(set_attr "type" "arith")])

;;
;; ....................
;;
;;  MOVE INSTRUCTIONS
;;
;; ....................

(define_insn "movsi"
  [(set (match_operand:SI 0 "general_operand" "=r,r,r,m")
	(match_operand:SI 1 "general_operand" "r,I,m,r"))]
  ""
  "@
   add\\t%0,r0,%1
   addi\\t%0,r0,%1
   lw\\t%0,%1
   sw\\t%1,%0"
  [(set_attr "type" "move,move,load,store")])

(define_insn "movhi"
  [(set (match_operand:HI 0 "general_operand" "=r,r,m")
	(match_operand:HI 1 "general_operand" "r,m,r"))]
  ""
  "@
   add\\t%0,r0,%1
   lh\\t%0,%1
   sh\\t%1,%0"
  [(set_attr "type" "move,load,store")])

(define_insn "movqi"
  [(set (match_operand:QI 0 "general_operand" "=r,r,m")
	(match_operand:QI 1 "general_operand" "r,m,r"))]
  ""
  "@
   add\\t%0,r0,%1
   lb\\t%0,%1
   sb\\t%1,%0"
  [(set_attr "type" "move,load,store")])

;; Floating point moves

(define_insn "movsf"
  [(set (match_operand:SF 0 "general_operand" "=f,f,m")
	(match_operand:SF 1 "general_operand" "f,m,f"))]
  "TARGET_FPU"
  "@
   fmovs\\t%0,%1
   lf\\t%0,%1
   sf\\t%1,%0"
  [(set_attr "type" "move,load,store")])

(define_insn "movdf"
  [(set (match_operand:DF 0 "general_operand" "=f,f,m")
	(match_operand:DF 1 "general_operand" "f,m,f"))]
  "TARGET_FPU"
  "@
   fmovd\\t%0,%1
   ld\\t%0,%1
   sd\\t%1,%0"
  [(set_attr "type" "move,load,store")])

;;
;; ....................
;;
;;  LOADS AND STORES
;;
;; ....................

;; Word loads

(define_insn ""
  [(set (match_operand:SI 0 "register_operand" "=r")
	(mem:SI (plus:SI (match_operand:SI 1 "register_operand" "r")
			 (match_operand:SI 2 "arith_operand" "I"))))]
  ""
  "lw\\t%0,%2(%1)"
  [(set_attr "type" "load")])

;; Word stores

(define_insn ""
  [(set (mem:SI (plus:SI (match_operand:SI 0 "register_operand" "r")
			 (match_operand:SI 1 "arith_operand" "I")))
	(match_operand:SI 2 "register_operand" "r"))]
  ""
  "sw\\t%2,%1(%0)"
  [(set_attr "type" "store")])

;;
;; ....................
;;
;;  COMPARISONS
;;
;; ....................

(define_expand "cmpsi"
  [(set (cc0)
	(compare (match_operand:SI 0 "register_operand" "")
		 (match_operand:SI 1 "arith_operand" "")))]
  ""
  "
{
  dlx_compare_op0 = operands[0];
  dlx_compare_op1 = operands[1];
  DONE;
}")

;;
;; ....................
;;
;;  BRANCHES
;;
;; ....................

(define_insn "beq"
  [(set (pc)
	(if_then_else (eq (cc0)
			  (const_int 0))
		      (label_ref (match_operand 0 "" ""))
		      (pc)))]
  ""
  "beqz\\t%0"
  [(set_attr "type" "branch")])

(define_insn "bne"
  [(set (pc)
	(if_then_else (ne (cc0)
			  (const_int 0))
		      (label_ref (match_operand 0 "" ""))
		      (pc)))]
  ""
  "bnez\\t%0"
  [(set_attr "type" "branch")])

(define_insn "bgt"
  [(set (pc)
	(if_then_else (gt (cc0)
			  (const_int 0))
		      (label_ref (match_operand 0 "" ""))
		      (pc)))]
  ""
  "bgtz\\t%0"
  [(set_attr "type" "branch")])

(define_insn "blt"
  [(set (pc)
	(if_then_else (lt (cc0)
			  (const_int 0))
		      (label_ref (match_operand 0 "" ""))
		      (pc)))]
  ""
  "bltz\\t%0"
  [(set_attr "type" "branch")])

(define_insn "bge"
  [(set (pc)
	(if_then_else (ge (cc0)
			  (const_int 0))
		      (label_ref (match_operand 0 "" ""))
		      (pc)))]
  ""
  "bgez\\t%0"
  [(set_attr "type" "branch")])

(define_insn "ble"
  [(set (pc)
	(if_then_else (le (cc0)
			  (const_int 0))
		      (label_ref (match_operand 0 "" ""))
		      (pc)))]
  ""
  "blez\\t%0"
  [(set_attr "type" "branch")])

;;
;; ....................
;;
;;  JUMPS
;;
;; ....................

(define_insn "jump"
  [(set (pc)
	(label_ref (match_operand 0 "" "")))]
  ""
  "j\\t%l0"
  [(set_attr "type" "jump")])

(define_insn "indirect_jump"
  [(set (pc) (match_operand:SI 0 "register_operand" "r"))]
  ""
  "jr\\t%0"
  [(set_attr "type" "jump")])

(define_insn "tablejump"
  [(set (pc)
	(match_operand:SI 0 "register_operand" "r"))
   (use (label_ref (match_operand 1 "" "")))]
  ""
  "jr\\t%0"
  [(set_attr "type" "jump")])

;;
;; ....................
;;
;;  FUNCTION CALLS
;;
;; ....................

(define_insn "call"
  [(call (match_operand 0 "call_operand" "m")
	 (match_operand 1 "" "i"))]
  ""
  "*
{
  if (GET_CODE (XEXP (operands[0], 0)) == REG)
    return \"jalr\\t%0\";
  else
    return \"jal\\t%0\";
}"
  [(set_attr "type" "call")])

(define_insn "call_value"
  [(set (match_operand 0 "register_operand" "=r")
	(call (match_operand 1 "call_operand" "m")
	      (match_operand 2 "" "i")))]
  ""
  "*
{
  if (GET_CODE (XEXP (operands[1], 0)) == REG)
    return \"jalr\\t%1\";
  else
    return \"jal\\t%1\";
}"
  [(set_attr "type" "call")])

;;
;; ....................
;;
;;  MISCELLANEOUS
;;
;; ....................

(define_insn "nop"
  [(const_int 0)]
  ""
  "nop"
  [(set_attr "type" "arith")])

;; Floating point arithmetic

(define_insn "addsf3"
  [(set (match_operand:SF 0 "register_operand" "=f")
	(plus:SF (match_operand:SF 1 "register_operand" "f")
		 (match_operand:SF 2 "register_operand" "f")))]
  "TARGET_FPU"
  "addf\\t%0,%1,%2"
  [(set_attr "type" "arith")])

(define_insn "adddf3"
  [(set (match_operand:DF 0 "register_operand" "=f")
	(plus:DF (match_operand:DF 1 "register_operand" "f")
		 (match_operand:DF 2 "register_operand" "f")))]
  "TARGET_FPU"
  "addd\\t%0,%1,%2"
  [(set_attr "type" "arith")])

(define_insn "subsf3"
  [(set (match_operand:SF 0 "register_operand" "=f")
	(minus:SF (match_operand:SF 1 "register_operand" "f")
		  (match_operand:SF 2 "register_operand" "f")))]
  "TARGET_FPU"
  "subf\\t%0,%1,%2"
  [(set_attr "type" "arith")])

(define_insn "subdf3"
  [(set (match_operand:DF 0 "register_operand" "=f")
	(minus:DF (match_operand:DF 1 "register_operand" "f")
		  (match_operand:DF 2 "register_operand" "f")))]
  "TARGET_FPU"
  "subd\\t%0,%1,%2"
  [(set_attr "type" "arith")])

(define_insn "mulsf3"
  [(set (match_operand:SF 0 "register_operand" "=f")
	(mult:SF (match_operand:SF 1 "register_operand" "f")
		 (match_operand:SF 2 "register_operand" "f")))]
  "TARGET_FPU"
  "multf\\t%0,%1,%2"
  [(set_attr "type" "arith")])

(define_insn "muldf3"
  [(set (match_operand:DF 0 "register_operand" "=f")
	(mult:DF (match_operand:DF 1 "register_operand" "f")
		 (match_operand:DF 2 "register_operand" "f")))]
  "TARGET_FPU"
  "multd\\t%0,%1,%2"
  [(set_attr "type" "arith")])

(define_insn "divsf3"
  [(set (match_operand:SF 0 "register_operand" "=f")
	(div:SF (match_operand:SF 1 "register_operand" "f")
		(match_operand:SF 2 "register_operand" "f")))]
  "TARGET_FPU"
  "divf\\t%0,%1,%2"
  [(set_attr "type" "arith")])

(define_insn "divdf3"
  [(set (match_operand:DF 0 "register_operand" "=f")
	(div:DF (match_operand:DF 1 "register_operand" "f")
		(match_operand:DF 2 "register_operand" "f")))]
  "TARGET_FPU"
  "divd\\t%0,%1,%2"
  [(set_attr "type" "arith")])
