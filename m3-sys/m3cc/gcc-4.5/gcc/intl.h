/* Modula-3: modified */

/* intl.h - internationalization
   Copyright 1998, 2001, 2003, 2004, 2007, 2009, 2010
   Free Software Foundation, Inc.

   GCC is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GCC is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GCC; see the file COPYING3.  If not see
   <http://www.gnu.org/licenses/>.  */

#ifndef GCC_INTL_H
#define GCC_INTL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Stubs.  */
# undef gettext
# define gettext(msgid) (msgid)
# define ngettext(singular,plural,n) fake_ngettext(singular,plural,n)

extern const char *fake_ngettext(const char *singular,const char *plural,
                                 unsigned long int n);

#ifndef _
# define _(msgid) gettext (msgid)
#endif

#ifndef N_
# define N_(msgid) msgid
#endif

#ifndef G_
# define G_(gmsgid) gmsgid
#endif

extern char *get_spaces (const char *);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* intl.h */
