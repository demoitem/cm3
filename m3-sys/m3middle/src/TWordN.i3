(* Copyright (C) 1993, Digital Equipment Corporation           *)
(* All rights reserved.                                        *)
(* See the file COPYRIGHT for a full description.              *)
(*                                                             *)
(* File: TWordN.i3                                             *)
(* Last Modified On Fri Nov 19 09:32:50 PST 1993 By kalsow     *)
(*      Modified On Thu May 20 08:20:38 PDT 1993 By muller     *)

INTERFACE TWordN; (* also known as TWord *)

(*  Modula-3 target description

    This interface provides simulations of the target machine's
    unsigned integer operations.

    Unless otherwise specified, the arithmetic operations defined
    below return TRUE if they succeed in producing a new target value,
    otherwise they return FALSE.
*)

FROM Target IMPORT IntN;

PROCEDURE Add (READONLY a, b: IntN;  VAR i: IntN);
(* returns 'Word.Plus (a, b)' *)

PROCEDURE Subtract (READONLY a, b: IntN;  VAR i: IntN);
(* returns 'Word.Minus (a, b)' *)

PROCEDURE Multiply (READONLY a, b: IntN;  VAR i: IntN);
(* returns 'Word.Times (a, b)' *)

PROCEDURE Div (READONLY a, b: IntN;  VAR i: IntN): BOOLEAN;
(* returns 'Word.Divide (a, b)' unless b is zero. *)

PROCEDURE Mod (READONLY a, b: IntN;  VAR i: IntN): BOOLEAN;
(* returns 'Word.Mod (a, b)' unless b is zero. *)

PROCEDURE DivMod (READONLY x, y: IntN;  VAR q, r: IntN);
(* returns 'q = x DIV y', and 'r = x MOD y', but assumes that 'y # 0' *)

PROCEDURE LT (READONLY a, b: IntN): BOOLEAN; (* a < b *)
PROCEDURE LE (READONLY a, b: IntN): BOOLEAN; (* a <= b *)
PROCEDURE EQ (READONLY a, b: IntN): BOOLEAN; (* a = b *)
PROCEDURE NE (READONLY a, b: IntN): BOOLEAN; (* a # b *)
PROCEDURE GE (READONLY a, b: IntN): BOOLEAN; (* a >= b *)
PROCEDURE GT (READONLY a, b: IntN): BOOLEAN; (* a > b *)

PROCEDURE And (READONLY a, b: IntN;  VAR i: IntN);
(* returns 'Word.And (a, b)' *)

PROCEDURE Or (READONLY a, b: IntN;  VAR i: IntN);
(* returns 'Word.Or (a, b)' *)

PROCEDURE Xor (READONLY a, b: IntN;  VAR i: IntN);
(* returns 'Word.Xor (a, b)' *)

PROCEDURE Not (READONLY a: IntN;  VAR i: IntN);
(* returns 'Word.Not (a)' *)

PROCEDURE Shift (READONLY x: IntN;  n: INTEGER;  VAR r: IntN);
(* returns 'Word.Shift (x, n)' *)

PROCEDURE LeftShift (READONLY x: IntN;  n: CARDINAL;  VAR r: IntN);
(* returns 'Word.LeftShift (x, n)' *)

PROCEDURE RightShift (READONLY x: IntN;  n: CARDINAL;  VAR r: IntN);
(* returns 'Word.RightShift (x, n)' *)

PROCEDURE Rotate (READONLY x: IntN;  n: INTEGER;  VAR r: IntN);
(* returns 'Word.Rotate (x, n)' *)

PROCEDURE Extract (READONLY x: IntN;  i, n: CARDINAL;  VAR r: IntN): BOOLEAN;
(* returns 'Word.Extract (x, i, n)' *)

PROCEDURE Insert (READONLY x, y: IntN;  i, n: CARDINAL;  VAR r: IntN): BOOLEAN;
(* returns 'Word.Insert (x, y, i, n)' *)

END TWordN.
