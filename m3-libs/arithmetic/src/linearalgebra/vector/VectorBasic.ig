GENERIC INTERFACE VectorBasic(R,VS);
(**Copyright (c) 1996, m3na project

Abstract: Vector math

2/17/96  Harry George    Convert from Objects to ADT's
**)
FROM NADefinitions IMPORT Error;
(*==========================*)

CONST
  Brand = R.Brand & "Vector";

TYPE
  (*text form: "V6{a0,a1,a2,a3,a4,a5}"*)
  TBody = ARRAY OF R.T;
  T     = REF TBody;

  TVBody = ARRAY OF T;

PROCEDURE New(n:CARDINAL):T; (*make new vector with n components T*)
PROCEDURE FromArray(READONLY x:TBody):T;
PROCEDURE FromVectorArray(READONLY x:TVBody):T;
PROCEDURE FromScalar (x: R.T): T;
PROCEDURE Copy(x:T):T;

(*
PROCEDURE Zero(x:T);                   (*set to zero*)
    (*NOTE: you should make unit vectors as needed*)
*)

PROCEDURE IsZero(x:T):BOOLEAN;
PROCEDURE Equal(x,y:T):BOOLEAN RAISES {Error};  (*return x=y*)

PROCEDURE Add(x,y:T):T RAISES {Error};   (*x+y*)
PROCEDURE Sub(x,y:T):T RAISES {Error};   (*x-y*)
PROCEDURE Neg(x:T):T;    (*return -x *)

PROCEDURE Scale(x:T; y:R.T):T;            (*x:=x*factor*)
PROCEDURE Inner(x,y:T):R.T RAISES {Error};   (*<x,y>*)
(* should be generalized to finding an orthonormal basis
   of the space orthogonal to a given set of vectors
PROCEDURE Cross(x,y:T):T RAISES {Error}; (*x x y*)
*)

PROCEDURE ArithSeq(num:CARDINAL;from:R.T;by:R.T):T;
PROCEDURE GeomSeq(num:CARDINAL;from:R.T;by:R.T):T;
PROCEDURE RecursiveSeq(num:CARDINAL;from:R.T;by:VS.MapFtn):T;

(*==========================*)
END VectorBasic.
