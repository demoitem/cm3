(* Copyright 1997, Critical Mass, Inc.  All rights reserved. *)

INTERFACE RTDebug;

IMPORT RT0;

PROCEDURE RegisterHandler (p: Handler);
(* Registers the procedures "p" to be called when <*DEBUG*> pragmas are
   executed.   The default handler prints its arguments. *)

TYPE
  Handler = PROCEDURE (m: RT0.ModulePtr; line: INTEGER;
                       READONLY msg: ARRAY OF TEXT) RAISES ANY;

END RTDebug.
