(* Copyright 1995-96 Critical Mass, Inc. All rights reserved.    *)

INTERFACE LexMisc;

IMPORT IP;

PROCEDURE ReadUID (READONLY buf: ARRAY OF CHAR;  VAR cursor: INTEGER): INTEGER;

PROCEDURE SkipBlanks (READONLY buf: ARRAY OF CHAR;  VAR cur: INTEGER);

PROCEDURE ReadName (READONLY buf: ARRAY OF CHAR;  VAR cursor: INTEGER): TEXT;

PROCEDURE ReadInt (READONLY buf: ARRAY OF CHAR;  VAR cursor: INTEGER): INTEGER;

PROCEDURE ReadBrand (READONLY buf: ARRAY OF CHAR; VAR cursor: INTEGER): TEXT;

PROCEDURE FmtUID (uid: INTEGER): TEXT;

PROCEDURE ScanUID (txt: TEXT): INTEGER;

PROCEDURE ScanInt (txt: TEXT): INTEGER;

PROCEDURE ScanIPAddress (txt: TEXT;  VAR(*OUT*) addr: IP.Address): BOOLEAN;

PROCEDURE ReadString (VAR buf: ARRAY OF CHAR): TEXT;

PROCEDURE ScanString (txt: TEXT): TEXT;
  
END LexMisc.
