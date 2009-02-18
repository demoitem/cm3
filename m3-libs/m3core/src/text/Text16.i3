(* Copyright 1996-2000, Critical Mass, Inc.  All rights reserved. *)
(* See file COPYRIGHT-CMASS for details. *)

(* A "Text16.T" is a text containing only characters of type WIDECHAR. *)

INTERFACE Text16;

TYPE
  T <: Public;
  Public = TEXT OBJECT
    contents: REF ARRAY OF WIDECHAR;
  END;
  (* The array contains the characters of the text followed by an
     element containing VAL(0,WIDECHAR). *)

PROCEDURE New (READONLY a: ARRAY OF WIDECHAR): TEXT;
(* Return a new text containing the characters of "a". *)

PROCEDURE Create (n: CARDINAL): T;
(* Create a new text capable of holding "n" characters. Note that the
   length of the "contents" array is "n+1" because of the null termination.
   The characters at positions "[0..n-1]" are left undefined. The character
   at position "n" is initialized to VAL(0,WIDECHAR) *)

END Text16.

