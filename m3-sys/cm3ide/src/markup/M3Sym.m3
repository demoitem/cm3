(* Copyright 1996, Critical Mass, Inc.  All rights reserved. *)

MODULE M3Sym;

IMPORT Buf, Text, Text2, M3Scanner;

FROM M3Scanner IMPORT
  TK_Module, TK_Interface, TK_Generic, TK_Procedure, TK_Ident,
  TK_Exports, TK_Semi, TK_Equal, TK_From, TK_Import, TK_As, TK_Comma,
  TK_Type, TK_Reveal, TK_Const, TK_Exception, TK_Var, TK_Subtype,
  TK_Value, TK_End, TK_Readonly, TK_For, TK_L_paren, TK_EOF,
  TK_R_paren, TK_Assign, TK_Object, TK_Branded, TK_Array, TK_Bits,
  TK_L_brace, TK_R_brace, TK_L_bracket, TK_R_bracket, TK_Record,
  TK_Methods, TK_Overrides, TK_Untraced, TK_Ref, TK_Set,
  TK_Colon, TK_Begin_pragma, TK_Dot, TK_Of, TK_Unsafe,
  TK_Card_const, TK_Real_const, TK_Longreal_const, TK_Extended_const,
  TK_Char_const, TK_Text_const, TK_Plus, TK_Minus, TK_Asterisk, 
  TK_Slash, TK_Ampersand, TK_Arrow, TK_Sharp, TK_Any, TK_Raises,
  TK_Less, TK_Greater, TK_Ls_equal, TK_Gr_equal, TK_Dot_dot;

TYPE
  State = RECORD
    cb        : CallBack;
    ignore    : KindSet;
    lex       : M3Scanner.T;
    unit      : Id;
    cur       : Id;
    qid_stack : ARRAY [0..19] OF Id;
  END;

EXCEPTION BailOut;

PROCEDURE Scan (buf: Buf.T;  cb: CallBack;  ignore: KindSet) =
  VAR s: State;
  BEGIN
    s.cb     := cb;
    s.ignore := ignore;
    s.lex    := NEW (M3Scanner.Default).initFromBuf (buf,
                     skip_comments := TRUE, split_pragmas := FALSE);
    TRY
      GetToken (s); (* prime the input stream *)
      Unit (s);
    EXCEPT BailOut =>
    END;
  END Scan;

PROCEDURE IgnoreSym (<*UNUSED*> self : CallBack;
            <*UNUSED*> READONLY sym  : Id;
                     <*UNUSED*> kind : Kind;
                     <*UNUSED*> intf : TEXT): BOOLEAN =
  BEGIN
    RETURN FALSE;
  END IgnoreSym;

PROCEDURE IgnoreQID (<*UNUSED*> self : CallBack;
            <*UNUSED*> READONLY sym  : QId;
                     <*UNUSED*> kind : Kind): BOOLEAN =
  BEGIN
    RETURN FALSE;
  END IgnoreQID;

PROCEDURE Unit (VAR s: State)  RAISES {BailOut} =
  CONST
    GenName = ARRAY BOOLEAN OF Kind { Kind.GImplName, Kind.GIntfName };
    RegName = ARRAY BOOLEAN OF Kind { Kind.ImplName,  Kind.IntfName  };
    GenUse  = ARRAY BOOLEAN OF Kind { Kind.GImplUse,  Kind.GIntfUse  };
  VAR
    is_intf: BOOLEAN;
  BEGIN
    IF (s.lex.token = TK_Unsafe) THEN GetToken (s); END;
    IF (s.lex.token = TK_Generic) THEN
      GetToken (s);  (* GENERIC *)
      is_intf := UnitKind (s);  (* INTERFACE / MODULE *)
      s.unit := s.cur;
      Note (s, GenName [is_intf]);
      GenericArgs (s, formals := TRUE);
      Match (s, TK_Semi);
      UnitBody (s);
    ELSE
      is_intf := UnitKind (s);  (* INTERFACE / MODULE *)
      s.unit := s.cur;
      Note (s, RegName [is_intf]);
      IF NOT is_intf THEN Exports (s); END;

      IF (s.lex.token = TK_Semi) THEN
        GetToken (s);  (* ; *)
        UnitBody (s);
      ELSIF (s.lex.token = TK_Equal) THEN
        GetToken (s);  (* = *)
        Note (s, GenUse [is_intf]);  (* generic's name *)
        GenericArgs (s, formals := FALSE);
        (*** Match (s, TK_End); ***)
      ELSE (* ERROR *)
        UnitBody (s);
      END;
    END;

    (****
    Match (s, TK_Dot);
    Match (s, TK_EOF);
    ****)
    WHILE (s.lex.token # TK_EOF) DO GetToken (s); END;
  END Unit;

PROCEDURE UnitKind (VAR s: State): BOOLEAN   RAISES {BailOut} =
  BEGIN
    IF (s.lex.token = TK_Interface) THEN
      GetToken (s);  (* INTERFACE *)
      RETURN TRUE;
    ELSIF (s.lex.token = TK_Module) THEN
      GetToken (s);  (* MODULE *)
      RETURN FALSE;
    ELSE (* ERROR *)
      RETURN FALSE;
    END;
  END UnitKind;

PROCEDURE UnitBody (VAR s: State)  RAISES {BailOut} =
  BEGIN
    Imports (s);
    Block (s);
  END UnitBody;

PROCEDURE GenericArgs (VAR s: State;  formals: BOOLEAN)  RAISES {BailOut} =
  CONST Map = ARRAY BOOLEAN OF Kind { Kind.GActual, Kind.GFormal };
  BEGIN
    Match (s, TK_L_paren);
    WHILE (s.lex.token = TK_Ident) DO
      Note (s, Map [formals]);
      IF (s.lex.token # TK_Comma) THEN EXIT; END;
      GetToken (s); (* , *)
    END;
    Match (s, TK_R_paren);
  END GenericArgs;

PROCEDURE Exports (VAR s: State)  RAISES {BailOut} =
  BEGIN
    IF (s.lex.token = TK_Exports) THEN
      GetToken (s);  (* EXPORTS *)
      WHILE (s.lex.token = TK_Ident) DO
        Note (s, Kind.Export);
        IF (s.lex.token # TK_Comma) THEN EXIT; END;
        GetToken (s); (* , *)
      END;
    ELSE (* no EXPORTs clause => Module name = EXPORT interface *)
      NoteSym (s, s.unit, Kind.Export);
    END;
  END Exports;

PROCEDURE Imports (VAR s: State)  RAISES {BailOut} =
  VAR nm: Id;  intf: TEXT;
  BEGIN
    LOOP
      IF (s.lex.token = TK_Import) THEN
        GetToken (s); (* IMPORT *)
        WHILE (s.lex.token = TK_Ident) DO
          nm := s.cur;
          GetToken (s); (* ID *)
          IF (s.lex.token = TK_As) THEN
            NoteSym (s, nm, Kind.ImportXX);
            GetToken (s); (* AS *)
            NoteAux (s, Kind.ImportAs, GetID (s, nm));
          ELSE
            NoteSym (s, nm, Kind.Import);
          END;
          IF (s.lex.token # TK_Comma) THEN EXIT; END;
          GetToken (s); (* , *)
        END;
        Match (s, TK_Semi);

      ELSIF (s.lex.token = TK_From) THEN
        GetToken (s); (* FROM *)
        intf := GetID (s, s.cur);
        Note (s, Kind.FromImport);
        Match (s, TK_Import);
        WHILE (s.lex.token = TK_Ident) DO
          NoteAux (s, Kind.SymImport, intf);
          IF (s.lex.token # TK_Comma) THEN EXIT; END;
          GetToken (s); (* , *)
        END;
        Match (s, TK_Semi);

      ELSE
        EXIT;
      END;
    END;
  END Imports;

PROCEDURE Block (VAR s: State)  RAISES {BailOut} =
  (* this parse is *very* sloppy! *)
  BEGIN
    LOOP
      CASE s.lex.token OF
      | TK_Const         => ConstDecl (s);
      | TK_Type          => TypeDecl (s);
      | TK_Var           => VarDecl (s);
      | TK_Procedure     => ProcDecl (s);
      | TK_Reveal        => Reveal (s);
      | TK_Exception     => ExceptDecl (s);
      | TK_EOF           => EXIT;
      ELSE                  GetToken (s);  (* toss it... *)
      END;
    END;
  END Block;

PROCEDURE ConstDecl (VAR s: State)  RAISES {BailOut} =
  BEGIN
    Match (s, TK_Const);
    WHILE (s.lex.token = TK_Ident) DO
      Note (s, Kind.ConstDecl);
      IF (s.lex.token = TK_Colon) THEN
        GetToken (s); (* : *)
        Type (s);
      END;
      Match (s, TK_Equal);
      Expr (s);
      Match (s, TK_Semi);
    END;
  END ConstDecl;

PROCEDURE TypeDecl (VAR s: State)  RAISES {BailOut} =
  BEGIN
    Match (s, TK_Type);
    WHILE (s.lex.token = TK_Ident) DO
      Note (s, Kind.TypeDecl);
      IF (s.lex.token = TK_Equal) OR (s.lex.token = TK_Subtype) THEN
        GetToken (s); (* = or <: *)
      END;
      Type (s);
      Match (s, TK_Semi);
    END;
  END TypeDecl;

PROCEDURE VarDecl (VAR s: State)  RAISES {BailOut} =
  BEGIN
    Match (s, TK_Var);
    WHILE (s.lex.token = TK_Ident) DO
      WHILE (s.lex.token = TK_Ident) DO
        Note (s, Kind.VarDecl);
        IF (s.lex.token # TK_Comma) THEN EXIT; END;
        GetToken (s); (* , *)
      END;
      IF (s.lex.token = TK_Colon) THEN
        GetToken (s); (* : *)
        Type (s);
      END;
      IF (s.lex.token = TK_Assign) THEN
        GetToken (s); (* := *)
        Expr (s);
      END;
      Match (s, TK_Semi);
    END;
  END VarDecl;

PROCEDURE ProcDecl (VAR s: State)  RAISES {BailOut} =
  VAR proc_id: Id;
  BEGIN
    Match (s, TK_Procedure);
    proc_id := s.cur;
    Note (s, Kind.ProcDecl);
    ProcSignature (s);
    IF (s.lex.token = TK_Equal) THEN
      GetToken (s); (* = *)
      ProcBody (s, proc_id);
    END;
    Match (s, TK_Semi);
  END ProcDecl;

PROCEDURE ProcBody (VAR s: State;  READONLY proc_id: Id)  RAISES {BailOut} =
  (* skip over the body *)
  VAR last_tok := TK_EOF;
  BEGIN
    LOOP
      CASE s.lex.token OF
      | TK_Ident =>
          IF    (last_tok = TK_End)
            AND (proc_id.len = s.cur.len)
            AND SUBARRAY (s.lex.buffer^, proc_id.start, proc_id.len)
                  = SUBARRAY (s.lex.buffer^, s.cur.start, s.cur.len) THEN
            GetToken (s); (* ID *)
            EXIT;
          END;
      | TK_EOF => EXIT;
      ELSE (* skip it *)
      END;
      last_tok := s.lex.token;
      GetToken (s);
    END;
  END ProcBody;

PROCEDURE Reveal (VAR s: State)  RAISES {BailOut} =
  BEGIN
    Match (s, TK_Reveal);
    WHILE (s.lex.token = TK_Ident) DO
      QID (s, Kind.TypeUse);
      IF (s.lex.token = TK_Equal) OR (s.lex.token = TK_Subtype) THEN
        GetToken (s); (* = or <: *)
      END;
      Type (s);
      Match (s, TK_Semi);
    END;
  END Reveal;

PROCEDURE ExceptDecl (VAR s: State)  RAISES {BailOut} =
  BEGIN
    Match (s, TK_Exception);
    WHILE (s.lex.token = TK_Ident) DO
      Note (s, Kind.ExceptDecl);
      IF (s.lex.token = TK_L_paren) THEN
        GetToken (s);  (* ( *)
        Type (s);
        Match (s, TK_R_paren);
      END;
      Match (s, TK_Semi);
    END;
  END ExceptDecl;

PROCEDURE Type (VAR s: State)  RAISES {BailOut} =
  BEGIN
    CASE s.lex.token OF
    | TK_Ident =>
        QID (s, Kind.TypeUse);
        IF (s.lex.token = TK_Object) OR (s.lex.token = TK_Branded) THEN
          Type (s);
        END;
    | TK_Array =>
        WHILE (s.lex.token # TK_Of) AND (s.lex.token # TK_EOF) DO
          GetToken (s); (* ARRAY or COMMA *)
          IF (s.lex.token # TK_Of) THEN Type (s); END;
        END;
        Match (s, TK_Of);
        Type (s);
    | TK_Bits =>
        Match (s, TK_For);
        Type (s);
    | TK_L_brace => (* skip enums *)
        Match (s, TK_R_brace);
    | TK_L_bracket => (* skip subranges *)
        Match (s, TK_R_bracket);
    | TK_Procedure =>
        GetToken (s); (* PROCEDURE *)
        ProcSignature (s);
    | TK_Record =>
        GetToken (s); (* RECORD *)
        Fields (s);
        Match (s, TK_End);
    | TK_Object => 
        GetToken (s); (* OBJECT *)
        Fields (s);
        IF (s.lex.token = TK_Methods) THEN
          GetToken (s); (* METHODS *)
          Methods (s);
        END;
        IF (s.lex.token = TK_Overrides) THEN
          GetToken (s); (* OVERRIDES *)
          Overrides (s);
        END;
        Match (s, TK_End);
        IF (s.lex.token = TK_Branded) OR (s.lex.token = TK_Object) THEN
          Type (s);
        END;
    | TK_Untraced =>
        GetToken (s); (* UNTRACED *)
        Type (s);
    | TK_Branded =>
        WHILE (s.lex.token # TK_Ref) AND (s.lex.token # TK_Object)
          AND (s.lex.token # TK_EOF) DO
          GetToken (s); (* skip the brand expression *)
        END;
        Type (s);
    | TK_Ref =>
        GetToken (s); (* REF *)
        Type (s);
    | TK_Set =>
        GetToken (s); (* SET *)
        Match (s, TK_Of);
        Type (s);
    | TK_L_paren =>
        GetToken (s); (* L_paren *)
        Type (s);
        Match (s, TK_R_paren);
    ELSE
      (* just ignore the garbage *)
    END;
  END Type;

PROCEDURE ProcSignature (VAR s: State)  RAISES {BailOut} =
  BEGIN
    Match (s, TK_L_paren);
    WHILE (s.lex.token # TK_R_paren) AND (s.lex.token # TK_EOF) DO
      CASE s.lex.token OF
      | TK_Var, TK_Readonly, TK_Value, TK_Semi =>
          (* skip *)
      | TK_Ident =>
          WHILE (s.lex.token # TK_Colon)
            AND (s.lex.token # TK_Assign)
            AND (s.lex.token # TK_R_paren)
            AND (s.lex.token # TK_EOF) DO
            GetToken (s); (* formal names and commas *)
          END;
          IF s.lex.token = TK_Colon THEN
            GetToken (s);  (* ':' *)
            Type (s);
          END;
          IF s.lex.token = TK_Assign THEN
            GetToken (s); (* ':=' *)
            Expr (s);
          END;
          WHILE (s.lex.token # TK_Semi) AND (s.lex.token # TK_R_paren)
            AND (s.lex.token # TK_EOF) DO
            GetToken (s);  (* discard *)
          END;
      ELSE
        (* just ignore in every-day use *)
      END;
      IF (s.lex.token # TK_R_paren) THEN GetToken (s); END;
    END;
    Match (s, TK_R_paren);
    IF (s.lex.token = TK_Colon) THEN
      GetToken (s); (* colon *)
      Type (s);
    END;
    IF (s.lex.token = TK_Raises) THEN
      GetToken (s); (* RAISES *)
      IF (s.lex.token = TK_Any) THEN
        GetToken (s); (* ANY *)
      ELSIF (s.lex.token = TK_L_brace) THEN
        GetToken (s); (* '{' *)
        WHILE (s.lex.token = TK_Ident) DO
          QID (s, Kind.ExceptUse);
          IF (s.lex.token # TK_Comma) THEN EXIT; END;
          GetToken (s); (* ',' *)
        END;
        Match (s, TK_R_brace);
      END;
    END;
    WHILE (s.lex.token # TK_Semi) AND (s.lex.token # TK_Equal)
      AND (s.lex.token # TK_R_paren) AND (s.lex.token # TK_Assign)
      AND (s.lex.token # TK_End) AND (s.lex.token # TK_EOF) DO
      GetToken (s);
    END;
  END ProcSignature;

PROCEDURE Fields (VAR s: State)  RAISES {BailOut} =
  BEGIN
    WHILE (s.lex.token # TK_Methods)
      AND (s.lex.token # TK_Overrides)
      AND (s.lex.token # TK_End)
      AND (s.lex.token # TK_EOF) DO
      WHILE (s.lex.token # TK_Colon)
        AND (s.lex.token # TK_Assign)
        AND (s.lex.token # TK_EOF) DO
        GetToken (s);
      END;
      IF s.lex.token = TK_Colon THEN
        GetToken (s);
        Type (s);
      END;
      IF s.lex.token = TK_Assign THEN
        GetToken (s); (* ':=' *)
        Expr (s);
      END;
      WHILE (s.lex.token # TK_Semi) AND (s.lex.token # TK_Methods)
        AND (s.lex.token # TK_Overrides) AND (s.lex.token # TK_End)
        AND (s.lex.token # TK_EOF) DO
        GetToken (s);
      END;
      IF (s.lex.token = TK_Semi) THEN GetToken (s); END;
    END;
  END Fields;

PROCEDURE Methods (VAR s: State)  RAISES {BailOut} =
  BEGIN
    WHILE (s.lex.token # TK_Overrides) AND (s.lex.token # TK_End)
      AND (s.lex.token # TK_EOF) DO
      GetToken (s);  (* skip ident *)
      ProcSignature (s);
      IF (s.lex.token = TK_Assign) THEN
        GetToken (s);
        QID (s, Kind.ProcUse);
      END;
      IF (s.lex.token = TK_Semi) THEN GetToken (s); END;
    END;
  END Methods;

PROCEDURE Overrides (VAR s: State)  RAISES {BailOut} =
  BEGIN
    WHILE (s.lex.token # TK_End) AND (s.lex.token # TK_EOF) DO
      Match (s, TK_Ident);
      Match (s, TK_Assign);
      QID (s, Kind.ProcUse);
      IF (s.lex.token = TK_Semi) THEN GetToken (s); END;
    END;
  END Overrides; 
  
PROCEDURE Expr (VAR s: State)  RAISES {BailOut} =
  VAR nest_cnt : INTEGER := 0;
  BEGIN
    LOOP
      CASE s.lex.token OF
      | TK_EOF =>
          EXIT;
      | TK_Ident =>
          QID (s, Kind.MiscUse);
      | TK_Array, TK_Bits, TK_Branded, TK_Object, TK_Procedure,
        TK_Record, TK_Ref, TK_Set, TK_Untraced =>
          Type (s);
      | TK_L_paren, TK_L_bracket, TK_L_brace =>
          INC (nest_cnt);
          GetToken (s);
      | TK_R_paren, TK_R_bracket, TK_R_brace =>
          IF (nest_cnt <= 0) THEN EXIT; END;
          DEC (nest_cnt);
          GetToken (s);
          IF (nest_cnt <= 0) THEN EXIT; END;
      | TK_Card_const, TK_Real_const, TK_Longreal_const, TK_Extended_const,
        TK_Char_const, TK_Text_const, TK_Plus, TK_Minus, TK_Asterisk, 
        TK_Slash, TK_Assign, TK_Ampersand, TK_Dot, TK_Comma, TK_Arrow,
        TK_Equal, TK_Sharp, TK_Less, TK_Greater, TK_Ls_equal, TK_Gr_equal,
        TK_Dot_dot =>
          GetToken (s);
      ELSE EXIT;
      END;
    END;
  END Expr;
  
PROCEDURE QID (VAR s: State;  kind: Kind)  RAISES {BailOut} =
  VAR n_stacked := 0;
  BEGIN
    IF (s.lex.token # TK_Ident) THEN RETURN END;

    IF IsPredefined (s) THEN RETURN; END;

    WHILE (s.lex.token = TK_Ident) DO
      IF (n_stacked < NUMBER (s.qid_stack)) THEN
        s.qid_stack[n_stacked] := s.cur;  INC (n_stacked);
      END;
      GetToken (s);  (* ID *)
      IF (s.lex.token # TK_Dot) THEN EXIT; END;
      GetToken (s); (* '.' *)
    END;

    IF (n_stacked = 1) THEN
      NoteSym (s, s.qid_stack[0], kind);
    ELSIF (n_stacked > 1) THEN
      IF NOT kind IN s.ignore THEN
        IF s.cb.note_qid (SUBARRAY (s.qid_stack, 0, n_stacked), kind) THEN
          RAISE BailOut;
        END;
      END;
    END;
  END QID;

TYPE
 XX = RECORD name: TEXT;  kind: Kind;  END;
CONST
  Predefined = ARRAY OF XX {
    XX { "ABS",       Kind.BuiltinOp },
    XX { "ADDRESS",   Kind.BuiltinType },
    XX { "ADR",       Kind.BuiltinOp },
    XX { "ADRSIZE",   Kind.BuiltinOp },
    XX { "BITSIZE",   Kind.BuiltinOp },
    XX { "BOOLEAN",   Kind.BuiltinType },
    XX { "BYTESIZE",  Kind.BuiltinOp },
    XX { "CARDINAL",  Kind.BuiltinType },
    XX { "CEILING",   Kind.BuiltinOp },
    XX { "CHAR",      Kind.BuiltinType },
    XX { "DEC",       Kind.BuiltinOp },
    XX { "DISPOSE",   Kind.BuiltinOp },
    XX { "EXTENDED",  Kind.BuiltinType },
    XX { "FALSE",     Kind.BuiltinConst },
    XX { "FIRST",     Kind.BuiltinOp },
    XX { "FLOAT",     Kind.BuiltinOp },
    XX { "FLOOR",     Kind.BuiltinOp },
    XX { "INC",       Kind.BuiltinOp },
    XX { "INTEGER",   Kind.BuiltinType },
    XX { "ISTYPE",    Kind.BuiltinOp },
    XX { "LAST",      Kind.BuiltinOp },
    XX { "LONGREAL",  Kind.BuiltinType },
    XX { "LOOPHOLE",  Kind.BuiltinOp },
    XX { "MAX",       Kind.BuiltinOp },
    XX { "MIN",       Kind.BuiltinOp },
    XX { "MUTEX",     Kind.BuiltinType },
    XX { "NARROW",    Kind.BuiltinOp },
    XX { "NEW",       Kind.BuiltinOp },
    XX { "NIL",       Kind.BuiltinConst },
    XX { "NULL",      Kind.BuiltinType },
    XX { "NUMBER",    Kind.BuiltinOp },
    XX { "ORD",       Kind.BuiltinOp },
    XX { "REAL",      Kind.BuiltinType },
    XX { "REFANY",    Kind.BuiltinType },
    XX { "ROOT",      Kind.BuiltinType },
    XX { "ROUND",     Kind.BuiltinOp },
    XX { "SUBARRAY",  Kind.BuiltinOp },
    XX { "TEXT",      Kind.BuiltinType },
    XX { "TRUE",      Kind.BuiltinConst },
    XX { "TRUNC",     Kind.BuiltinOp },
    XX { "TYPECODE",  Kind.BuiltinOp },
    XX { "VAL",       Kind.BuiltinOp }
  };

PROCEDURE IsPredefined (VAR s: State): BOOLEAN  RAISES {BailOut}  =
  BEGIN
    IF (s.cur.len < 3) OR (s.cur.len > 8) THEN RETURN FALSE; END;
    WITH id = SUBARRAY (s.lex.buffer^, s.cur.start, s.cur.len) DO
      FOR i := FIRST (Predefined) TO LAST (Predefined) DO
        IF Text2.EqualSub (Predefined[i].name, id) THEN
          Note (s, Predefined[i].kind);
          RETURN TRUE;
        END;
      END;
    END;
    RETURN FALSE;
  END IsPredefined;

PROCEDURE Match (VAR s: State;  token: M3Scanner.TK)  RAISES {BailOut} =
  BEGIN
    WHILE (s.lex.token # token) AND (s.lex.token # TK_EOF) DO GetToken (s); END;
    GetToken (s);
  END Match;

PROCEDURE GetID (VAR s: State;  READONLY sym: Id): TEXT  =
  BEGIN
    IF (s.lex.token # TK_Ident) THEN RETURN ""; END;
    RETURN Text.FromChars (SUBARRAY (s.lex.buffer^, sym.start, sym.len));
  END GetID;

PROCEDURE GetToken (VAR s: State)  RAISES {BailOut} =
  BEGIN
    IF    (NOT Kind.Keyword IN s.ignore)
      AND (M3Scanner.First_Keyword <= s.lex.token)
      AND (s.lex.token <= M3Scanner.Last_Keyword) THEN
      IF s.cb.note_sym (s.cur, Kind.Keyword, NIL) THEN RAISE BailOut; END;
    END;

    REPEAT
      s.lex.next ();
    UNTIL (s.lex.token # TK_Begin_pragma);

    s.cur.start := s.lex.offset;
    s.cur.len   := s.lex.length;
  END GetToken;

PROCEDURE Note (VAR s: State;  kind: Kind)  RAISES {BailOut} =
  BEGIN
    IF (s.lex.token = TK_Ident) THEN
      IF NOT kind IN s.ignore THEN
        IF s.cb.note_sym (s.cur, kind, NIL) THEN RAISE BailOut; END;
      END;
      GetToken (s);  (* ID *)
    END;
  END Note;

PROCEDURE NoteAux (VAR s: State;  kind: Kind;  intf: TEXT) RAISES {BailOut} =
  BEGIN
    IF (s.lex.token = TK_Ident) THEN
      IF NOT kind IN s.ignore THEN
        IF s.cb.note_sym (s.cur, kind, intf) THEN RAISE BailOut; END;
      END;
      GetToken (s);  (* ID *)
    END;
  END NoteAux;

PROCEDURE NoteSym (VAR s: State;  READONLY sym: Id;  kind: Kind)  RAISES {BailOut} =
  BEGIN
    IF NOT kind IN s.ignore THEN
      IF s.cb.note_sym (sym, kind, NIL) THEN RAISE BailOut; END;
    END;
  END NoteSym;

BEGIN 
END M3Sym.
