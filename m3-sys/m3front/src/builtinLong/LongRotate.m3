(* Copyright (C) 1992, Digital Equipment Corporation           *)
(* All rights reserved.                                        *)
(* See the file COPYRIGHT for a full description.              *)

(* File: LongRotate.i3                                         *)
(* Last Modified On Mon Dec  5 15:30:43 PST 1994 By kalsow     *)
(*      Modified On Thu May 17 08:59:11 1990 By muller         *)

MODULE LongRotate;

IMPORT CG, CallExpr, Expr, ExprRep, Procedure, Formal, SubrangeType;
IMPORT LInt, IntegerExpr, Value, ProcType, CheckExpr, Target, TInt, TWord;
IMPORT Int;

VAR Z, ZL, ZR: CallExpr.MethodList;
VAR formals, formalsL, formalsR: Value.T;

PROCEDURE Check (ce: CallExpr.T;  VAR cs: Expr.CheckState) =
  BEGIN
    EVAL Formal.CheckArgs (cs, ce.args, formals, ce.proc);
    ce.type := LInt.T;
  END Check;

PROCEDURE CheckL (ce: CallExpr.T;  VAR cs: Expr.CheckState) =
  BEGIN
    EVAL Formal.CheckArgs (cs, ce.args, formalsL, ce.proc);
    ce.type := LInt.T;
  END CheckL;

PROCEDURE CheckR (ce: CallExpr.T;  VAR cs: Expr.CheckState) =
  BEGIN
    EVAL Formal.CheckArgs (cs, ce.args, formalsR, ce.proc);
    ce.type := LInt.T;
  END CheckR;

PROCEDURE Compile (ce: CallExpr.T) =
  BEGIN
    Expr.Compile (ce.args[0]);
    Expr.Compile (ce.args[1]);
    CG.Rotate (Target.Longint.cg_type);
  END Compile;

PROCEDURE CompileL (ce: CallExpr.T) =
  VAR max: Target.Int;
      b := TInt.FromInt (Target.Longint.size -1, Target.Pre.Integer, max);
  BEGIN
    <* ASSERT b *>
    Expr.Compile (ce.args[0]);
    CheckExpr.EmitChecks (ce.args[1], TInt.Zero, max,
                          CG.RuntimeError.ValueOutOfRange);
    CG.Rotate_left (Target.Longint.cg_type);
  END CompileL;

PROCEDURE CompileR (ce: CallExpr.T) =
  VAR max: Target.Int;
      b := TInt.FromInt (Target.Longint.size -1, Target.Pre.Integer, max);
  BEGIN
    <* ASSERT b *>
    Expr.Compile (ce.args[0]);
    CheckExpr.EmitChecks (ce.args[1], TInt.Zero, max,
                          CG.RuntimeError.ValueOutOfRange);
    CG.Rotate_right (Target.Longint.cg_type);
  END CompileR;

PROCEDURE Fold (ce: CallExpr.T): Expr.T =
  VAR e0, e1: Expr.T;  w0, i1, result: Target.Int;
  BEGIN
    e0 := Expr.ConstValue (ce.args[0]);
    e1 := Expr.ConstValue (ce.args[1]);
    IF (e0 # NIL) AND IntegerExpr.Split (e0, w0)
      AND (e1 # NIL) AND IntegerExpr.Split (e1, i1)
    THEN
      TWord.Rotate (w0, i1, result);
      RETURN IntegerExpr.New (result);
    ELSE
      RETURN NIL;
    END;
  END Fold;

PROCEDURE FoldL (ce: CallExpr.T): Expr.T =
  VAR e0, e1: Expr.T;  w0, i1, max, result: Target.Int;
  BEGIN
    e0 := Expr.ConstValue (ce.args[0]);
    e1 := Expr.ConstValue (ce.args[1]);
    IF (e0 # NIL) AND IntegerExpr.Split (e0, w0)
      AND (e1 # NIL) AND IntegerExpr.Split (e1, i1)
      AND TInt.LE (TInt.Zero, i1)
      AND TInt.FromInt (Target.Longint.size, Target.Pre.Integer, max)
      AND TInt.LT (i1, max)
    THEN
      TWord.Rotate (w0, i1, result);
      RETURN IntegerExpr.New (result);
    ELSE
      RETURN NIL;
    END;
  END FoldL;

PROCEDURE FoldR (ce: CallExpr.T): Expr.T =
  VAR e0, e1: Expr.T;  w0, i1, max, neg_i1, result: Target.Int;
  BEGIN
    e0 := Expr.ConstValue (ce.args[0]);
    e1 := Expr.ConstValue (ce.args[1]);
    IF (e0 # NIL) AND IntegerExpr.Split (e0, w0)
      AND (e1 # NIL) AND IntegerExpr.Split (e1, i1)
      AND TInt.LE (TInt.Zero, i1)
      AND TInt.FromInt (Target.Longint.size, Target.Pre.Integer, max)
      AND TInt.LT (i1, max)
      AND TInt.Subtract (TInt.Zero, i1, neg_i1)
    THEN
      TWord.Rotate (w0, neg_i1, result);
      RETURN IntegerExpr.New (result);
    ELSE
      RETURN NIL;
    END;
  END FoldR;

PROCEDURE Initialize () =
  VAR
    max : Target.Int;
    b   := TInt.FromInt (Target.Longint.size-1, Target.Pre.Integer, max);
    sub := SubrangeType.New (TInt.Zero, max, Int.T, FALSE);

    f0  := Formal.NewBuiltin ("x", 0, LInt.T);
    f1  := Formal.NewBuiltin ("n", 1, Int.T);
    t   := ProcType.New (LInt.T, f0, f1);

    Lf0 := Formal.NewBuiltin ("x", 0, LInt.T);
    Lf1 := Formal.NewBuiltin ("n", 1, sub);
    Lt  := ProcType.New (LInt.T, Lf0, Lf1);

    Rf0 := Formal.NewBuiltin ("x", 0, LInt.T);
    Rf1 := Formal.NewBuiltin ("n", 1, sub);
    Rt  := ProcType.New (LInt.T, Rf0, Rf1);
  BEGIN
    <*ASSERT b*>
    Z := CallExpr.NewMethodList (2, 2, TRUE, TRUE, TRUE, LInt.T,
                                 NIL,
                                 CallExpr.NotAddressable,
                                 Check,
                                 CallExpr.PrepArgs,
                                 Compile,
                                 CallExpr.NoLValue,
                                 CallExpr.NoLValue,
                                 CallExpr.NotBoolean,
                                 CallExpr.NotBoolean,
                                 Fold,
                                 CallExpr.NoBounds,
                                 CallExpr.IsNever, (* writable *)
                                 CallExpr.IsNever, (* designator *)
                                 CallExpr.NotWritable (* noteWriter *));
    Procedure.Define ("Rotate", Z, FALSE, t);
    formals := ProcType.Formals (t);

    ZL := CallExpr.NewMethodList (2, 2, TRUE, TRUE, TRUE, LInt.T,
                                 NIL,
                                 CallExpr.NotAddressable,
                                 CheckL,
                                 CallExpr.PrepArgs,
                                 CompileL,
                                 CallExpr.NoLValue,
                                 CallExpr.NoLValue,
                                 CallExpr.NotBoolean,
                                 CallExpr.NotBoolean,
                                 FoldL,
                                 CallExpr.NoBounds,
                                 CallExpr.IsNever, (* writable *)
                                 CallExpr.IsNever, (* designator *)
                                 CallExpr.NotWritable (* noteWriter *));
    Procedure.Define ("LeftRotate", ZL, FALSE, Lt);
    formalsL := ProcType.Formals (Lt);

    ZR := CallExpr.NewMethodList (2, 2, TRUE, TRUE, TRUE,  LInt.T,
                                 NIL,
                                 CallExpr.NotAddressable,
                                 CheckR,
                                 CallExpr.PrepArgs,
                                 CompileR,
                                 CallExpr.NoLValue,
                                 CallExpr.NoLValue,
                                 CallExpr.NotBoolean,
                                 CallExpr.NotBoolean,
                                 FoldR,
                                 CallExpr.NoBounds,
                                 CallExpr.IsNever, (* writable *)
                                 CallExpr.IsNever, (* designator *)
                                 CallExpr.NotWritable (* noteWriter *));
    Procedure.Define ("RightRotate", ZR, FALSE, Rt);
    formalsR := ProcType.Formals (Rt);
  END Initialize;

BEGIN
END LongRotate.
