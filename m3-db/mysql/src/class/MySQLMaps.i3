INTERFACE MySQLMaps;

(* This module returns Modula3 mappings for C structs returned by
   some mysql functions. 
*)

IMPORT MySQL;

TYPE

  M3Field = RECORD
             name             : TEXT;
             org_name         : TEXT;
             table            : TEXT;
             org_table        : TEXT;
             db               : TEXT;
             catalog          : TEXT;
             def              : TEXT;
             length           : CARDINAL;
             max_length       : CARDINAL;
             name_length      : CARDINAL;
             org_name_length  : CARDINAL;
             table_length     : CARDINAL;
             org_table_length : CARDINAL;
             db_length        : CARDINAL;
             catalog_length   : CARDINAL;
             def_length       : CARDINAL;
             flags            : CARDINAL;
             decimals         : CARDINAL;
             charsetnr        : CARDINAL;
             type             : CARDINAL;
            END;

  M3FieldRef   = REF M3Field;
  M3FieldArray = REF ARRAY OF M3FieldRef;


PROCEDURE Field(fieldRef : MySQL.FieldT) : M3FieldRef;

PROCEDURE FieldList(res : MySQL.ResT) : M3FieldArray;

PROCEDURE DumpField(m3field : M3FieldRef);

END MySQLMaps.
