/* Modula-3: modified */

/* Callgraph based interprocedural optimizations.
   Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010,
   2011 Free Software Foundation, Inc.
   Contributed by Jan Hubicka

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 3, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING3.  If not see
<http://www.gnu.org/licenses/>.  */

/* This module implements main driver of compilation process as well as
   few basic interprocedural optimizers.

   The main scope of this file is to act as an interface in between
   tree based frontends and the backend (and middle end)

   The front-end is supposed to use following functionality:

    - cgraph_finalize_function

      This function is called once front-end has parsed whole body of function
      and it is certain that the function body nor the declaration will change.

      (There is one exception needed for implementing GCC extern inline
	function.)

    - varpool_finalize_variable

      This function has same behavior as the above but is used for static
      variables.

    - cgraph_finalize_compilation_unit

      This function is called once (source level) compilation unit is finalized
      and it will no longer change.

      In the the call-graph construction and local function
      analysis takes place here.  Bodies of unreachable functions are released
      to conserve memory usage.

      The function can be called multiple times when multiple source level
      compilation units are combined (such as in C frontend)

    - cgraph_optimize

      In this unit-at-a-time compilation the intra procedural analysis takes
      place here.  In particular the static functions whose address is never
      taken are marked as local.  Backend can then use this information to
      modify calling conventions, do better inlining or similar optimizations.

    - cgraph_mark_needed_node
    - varpool_mark_needed_node

      When function or variable is referenced by some hidden way the call-graph
      data structure must be updated accordingly by this function.
      There should be little need to call this function and all the references
      should be made explicit to cgraph code.  At present these functions are
      used by C++ frontend to explicitly mark the keyed methods.

    - analyze_expr callback

      This function is responsible for lowering tree nodes not understood by
      generic code into understandable ones or alternatively marking
      callgraph and varpool nodes referenced by the as needed.

      ??? On the tree-ssa genericizing should take place here and we will avoid
      need for these hooks (replacing them by genericizing hook)

        Analyzing of all functions is deferred
	to cgraph_finalize_compilation_unit and expansion into cgraph_optimize.

	In cgraph_finalize_compilation_unit the reachable functions are
	analyzed.  During analysis the call-graph edges from reachable
	functions are constructed and their destinations are marked as
	reachable.  References to functions and variables are discovered too
	and variables found to be needed output to the assembly file.  Via
	mark_referenced call in assemble_variable functions referenced by
	static variables are noticed too.

	The intra-procedural information is produced and its existence
	indicated by global_info_ready.  Once this flag is set it is impossible
	to change function from !reachable to reachable and thus
	assemble_variable no longer call mark_referenced.

	Finally the call-graph is topologically sorted and all reachable functions
	that has not been completely inlined or are not external are output.

	??? It is possible that reference to function or variable is optimized
	out.  We can not deal with this nicely because topological order is not
	suitable for it.  For tree-ssa we may consider another pass doing
	optimization and re-discovering reachable functions.

	??? Reorganize code so variables are output very last and only if they
	really has been referenced by produced code, so we catch more cases
	where reference has been optimized out.  */


#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"
#include "rtl.h"
#include "tree-flow.h"
#include "tree-inline.h"
#include "langhooks.h"
#include "pointer-set.h"
#include "toplev.h"
#include "flags.h"
#include "ggc.h"
#include "debug.h"
#include "target.h"
#include "cgraph.h"
#include "diagnostic.h"
#include "tree-pretty-print.h"
#include "gimple-pretty-print.h"
#include "timevar.h"
#include "params.h"
#include "fibheap.h"
#include "intl.h"
#include "function.h"
#include "gimple.h"
#include "tree-iterator.h"
#include "tree-pass.h"
#include "tree-dump.h"
#include "output.h"
#include "coverage.h"
#include "plugin.h"

EXTERN_C_START

static void cgraph_expand_all_functions (void);
static void cgraph_mark_functions_to_output (void);
static void cgraph_expand_function (struct cgraph_node *);
static void cgraph_output_pending_asms (void);
static void cgraph_analyze_function (struct cgraph_node *);

FILE *cgraph_dump_file;

/* Used for vtable lookup in thunk adjusting.  */
static GTY (()) tree vtable_entry_type;

/* Process CGRAPH_NEW_FUNCTIONS and perform actions necessary to add these
   functions into callgraph in a way so they look like ordinary reachable
   functions inserted into callgraph already at construction time.  */

bool
cgraph_process_new_functions (void)
{
  bool output = false;
  tree fndecl;
  struct cgraph_node *node;

  varpool_analyze_pending_decls ();
  /*  Note that this queue may grow as its being processed, as the new
      functions may generate new ones.  */
  while (cgraph_new_nodes)
    {
      node = cgraph_new_nodes;
      fndecl = node->decl;
      cgraph_new_nodes = cgraph_new_nodes->next_needed;
      switch (cgraph_state)
	{
	case CGRAPH_STATE_CONSTRUCTION:
	  /* At construction time we just need to finalize function and move
	     it into reachable functions list.  */

	  node->next_needed = NULL;
	  cgraph_finalize_function (fndecl, false);
	  cgraph_mark_reachable_node (node);
	  output = true;
	  break;

	case CGRAPH_STATE_IPA:
	case CGRAPH_STATE_IPA_SSA:
	  /* When IPA optimization already started, do all essential
	     transformations that has been already performed on the whole
	     cgraph but not on this function.  */

	  gimple_register_cfg_hooks ();
	  if (!node->analyzed)
	    cgraph_analyze_function (node);
	  push_cfun (DECL_STRUCT_FUNCTION (fndecl));
	  current_function_decl = fndecl;
	  if ((cgraph_state == CGRAPH_STATE_IPA_SSA
	      && !gimple_in_ssa_p (DECL_STRUCT_FUNCTION (fndecl)))
	      /* When not optimizing, be sure we run early local passes anyway
		 to expand OMP.  */
	      || !optimize)
	    execute_pass_list (pass_early_local_passes.pass.sub);
	  else
	    gcc_unreachable ();
	  free_dominance_info (CDI_POST_DOMINATORS);
	  free_dominance_info (CDI_DOMINATORS);
	  pop_cfun ();
	  current_function_decl = NULL;
	  break;

	case CGRAPH_STATE_EXPANSION:
	  /* Functions created during expansion shall be compiled
	     directly.  */
	  node->process = 0;
	  cgraph_expand_function (node);
	  break;

	default:
	  gcc_unreachable ();
	  break;
	}
      cgraph_call_function_insertion_hooks (node);
      varpool_analyze_pending_decls ();
    }
  return output;
}

/* As an GCC extension we allow redefinition of the function.  The
   semantics when both copies of bodies differ is not well defined.
   We replace the old body with new body so in unit at a time mode
   we always use new body, while in normal mode we may end up with
   old body inlined into some functions and new body expanded and
   inlined in others.

   ??? It may make more sense to use one body for inlining and other
   body for expanding the function but this is difficult to do.  */

static void
cgraph_reset_node (struct cgraph_node *node)
{
  /* If node->process is set, then we have already begun whole-unit analysis.
     This is *not* testing for whether we've already emitted the function.
     That case can be sort-of legitimately seen with real function redefinition
     errors.  I would argue that the front end should never present us with
     such a case, but don't enforce that for now.  */
  gcc_assert (!node->process);

  /* Reset our data structures so we can analyze the function again.  */
  memset (&node->local, 0, sizeof (node->local));
  memset (&node->global, 0, sizeof (node->global));
  memset (&node->rtl, 0, sizeof (node->rtl));
  node->analyzed = false;
  node->local.redefined_extern_inline = true;
  node->local.finalized = false;

  cgraph_node_remove_callees (node);

  /* We may need to re-queue the node for assembling in case
     we already proceeded it and ignored as not needed or got
     a re-declaration in IMA mode.  */
  if (node->reachable)
    {
      struct cgraph_node *n;

      for (n = cgraph_nodes_queue; n; n = n->next_needed)
	if (n == node)
	  break;
      if (!n)
	node->reachable = 0;
    }
}

static void
cgraph_lower_function (struct cgraph_node *node)
{
  if (node->lowered)
    return;

  if (node->nested)
    lower_nested_functions (node->decl);
  gcc_assert (!node->nested);

  tree_lowering_passes (node->decl);
  node->lowered = true;
}

/* DECL has been parsed.  Take it, queue it, compile it at the whim of the
   logic in effect.  If NESTED is true, then our caller cannot stand to have
   the garbage collector run at the moment.  We would need to either create
   a new GC context, or just not compile right now.  */

void
cgraph_finalize_function (tree decl, bool nested)
{
  struct cgraph_node *node = cgraph_node (decl);

  if (node->local.finalized)
    cgraph_reset_node (node);

  node->pid = cgraph_max_pid ++;
  notice_global_symbol (decl);
  node->local.finalized = true;
  node->lowered = DECL_STRUCT_FUNCTION (decl)->cfg != NULL;
  node->finalized_by_frontend = true;

  cgraph_mark_needed_node (node);

  /* Since we reclaim unreachable nodes at the end of every language
     level unit, we need to be conservative about possible entry points
     there.  */
  if ((TREE_PUBLIC (decl) && !DECL_COMDAT (decl) && !DECL_EXTERNAL (decl))
      || DECL_STATIC_CONSTRUCTOR (decl)
      || DECL_STATIC_DESTRUCTOR (decl)
      /* COMDAT virtual functions may be referenced by vtable from
	 other compilation unit.  Still we want to devirtualize calls
	 to those so we need to analyze them.
	 FIXME: We should introduce may edges for this purpose and update
	 their handling in unreachable function removal and inliner too.  */
      || (DECL_VIRTUAL_P (decl) && (DECL_COMDAT (decl) || DECL_EXTERNAL (decl))))
    cgraph_mark_reachable_node (node);

  /* If we've not yet emitted decl, tell the debug info about it.  */
  if (!TREE_ASM_WRITTEN (decl))
    (*debug_hooks->deferred_inline_function) (decl);

  /* Possibly warn about unused parameters.  */
  if (warn_unused_parameter)
    do_warn_unused_parameter (decl);

  if (!nested)
    ggc_collect ();
}

/* C99 extern inline keywords allow changing of declaration after function
   has been finalized.  We need to re-decide if we want to mark the function as
   needed then.   */

void
cgraph_mark_if_needed (tree decl)
{
  struct cgraph_node *node = cgraph_node (decl);
  if (node->local.finalized)
    cgraph_mark_needed_node (node);
}

/* Return TRUE if NODE2 is equivalent to NODE or its clone.  */
static bool
clone_of_p (struct cgraph_node *node, struct cgraph_node *node2)
{
  while (node != node2 && node2)
    node2 = node2->clone_of;
  return node2 != NULL;
}

/* Verify edge E count and frequency.  */

static bool
verify_edge_count_and_frequency (struct cgraph_edge *e)
{
  bool error_found = false;
  if (e->count < 0)
    {
      error ("caller edge count is negative");
      error_found = true;
    }
  if (e->frequency < 0)
    {
      error ("caller edge frequency is negative");
      error_found = true;
    }
  if (e->frequency > CGRAPH_FREQ_MAX)
    {
      error ("caller edge frequency is too large");
      error_found = true;
    }
  if (gimple_has_body_p (e->caller->decl)
      && !e->caller->global.inlined_to
      && (e->frequency
	  != compute_call_stmt_bb_frequency (e->caller->decl,
					     gimple_bb (e->call_stmt))))
    {
      error ("caller edge frequency %i does not match BB frequency %i",
	     e->frequency,
	     compute_call_stmt_bb_frequency (e->caller->decl,
					     gimple_bb (e->call_stmt)));
      error_found = true;
    }
  return error_found;
}

/* Switch to THIS_CFUN if needed and print STMT to stderr.  */
static void
cgraph_debug_gimple_stmt (struct function *this_cfun, gimple stmt)
{
  /* debug_gimple_stmt needs correct cfun */
  if (cfun != this_cfun)
    set_cfun (this_cfun);
  debug_gimple_stmt (stmt);
}

/* Output all asm statements we have stored up to be output.  */

static void
cgraph_output_pending_asms (void)
{
  struct cgraph_asm_node *can;

  if (seen_error ())
    return;

  for (can = cgraph_asm_nodes; can; can = can->next)
    assemble_asm (can->asm_str);
  cgraph_asm_nodes = NULL;
}

/* Analyze the function scheduled to be output.  */
static void
cgraph_analyze_function (struct cgraph_node *node)
{
  tree save = current_function_decl;
  tree decl = node->decl;

  current_function_decl = decl;
  push_cfun (DECL_STRUCT_FUNCTION (decl));

  assign_assembler_name_if_neeeded (node->decl);

  /* disregard_inline_limits affects topological order of the early optimization,
     so we need to compute it ahead of rest of inline parameters.  */
  node->local.disregard_inline_limits
    = DECL_DISREGARD_INLINE_LIMITS (node->decl);

  /* Make sure to gimplify bodies only once.  During analyzing a
     function we lower it, which will require gimplified nested
     functions, so we can end up here with an already gimplified
     body.  */
  if (!gimple_body (decl))
    gimplify_function_tree (decl);
  dump_function (TDI_generic, decl);

  cgraph_lower_function (node);
  node->analyzed = true;

  pop_cfun ();
  current_function_decl = save;
}

/* Process attributes common for vars and functions.  */

static void
process_common_attributes (tree decl)
{
  tree weakref = lookup_attribute ("weakref", DECL_ATTRIBUTES (decl));

  if (weakref && !lookup_attribute ("alias", DECL_ATTRIBUTES (decl)))
    {
      warning_at (DECL_SOURCE_LOCATION (decl), OPT_Wattributes,
		  "%<weakref%> attribute should be accompanied with"
		  " an %<alias%> attribute");
      DECL_WEAK (decl) = 0;
      DECL_ATTRIBUTES (decl) = remove_attribute ("weakref",
						 DECL_ATTRIBUTES (decl));
    }
}

/* Look for externally_visible and used attributes and mark cgraph nodes
   accordingly.

   We cannot mark the nodes at the point the attributes are processed (in
   handle_*_attribute) because the copy of the declarations available at that
   point may not be canonical.  For example, in:

    void f();
    void f() __attribute__((used));

   the declaration we see in handle_used_attribute will be the second
   declaration -- but the front end will subsequently merge that declaration
   with the original declaration and discard the second declaration.

   Furthermore, we can't mark these nodes in cgraph_finalize_function because:

    void f() {}
    void f() __attribute__((externally_visible));

   is valid.

   So, we walk the nodes at the end of the translation unit, applying the
   attributes at that point.  */

static void
process_function_and_variable_attributes (struct cgraph_node *first,
                                          struct varpool_node *first_var)
{
  struct cgraph_node *node;
  struct varpool_node *vnode;

  for (node = cgraph_nodes; node != first; node = node->next)
    {
      tree decl = node->decl;
      if (DECL_PRESERVE_P (decl))
	cgraph_mark_needed_node (node);
      if (TARGET_DLLIMPORT_DECL_ATTRIBUTES
	  && lookup_attribute ("dllexport", DECL_ATTRIBUTES (decl))
	  && TREE_PUBLIC (node->decl))
	{
	  if (node->local.finalized)
	    cgraph_mark_needed_node (node);
	}
      else if (lookup_attribute ("externally_visible", DECL_ATTRIBUTES (decl)))
	{
	  if (! TREE_PUBLIC (node->decl))
	    warning_at (DECL_SOURCE_LOCATION (node->decl), OPT_Wattributes,
			"%<externally_visible%>"
			" attribute have effect only on public objects");
	  else if (node->local.finalized)
	     cgraph_mark_needed_node (node);
	}
      if (lookup_attribute ("weakref", DECL_ATTRIBUTES (decl))
	  && node->local.finalized)
	{
	  warning_at (DECL_SOURCE_LOCATION (node->decl), OPT_Wattributes,
		      "%<weakref%> attribute ignored"
		      " because function is defined");
	  DECL_WEAK (decl) = 0;
	  DECL_ATTRIBUTES (decl) = remove_attribute ("weakref",
						     DECL_ATTRIBUTES (decl));
	}
      process_common_attributes (decl);
    }
  for (vnode = varpool_nodes; vnode != first_var; vnode = vnode->next)
    {
      tree decl = vnode->decl;
      if (DECL_PRESERVE_P (decl))
	{
	  vnode->force_output = true;
	  if (vnode->finalized)
	    varpool_mark_needed_node (vnode);
	}
      if (TARGET_DLLIMPORT_DECL_ATTRIBUTES
	  && lookup_attribute ("dllexport", DECL_ATTRIBUTES (decl))
	  && TREE_PUBLIC (vnode->decl))
	{
	  if (vnode->finalized)
	    varpool_mark_needed_node (vnode);
	}
      else if (lookup_attribute ("externally_visible", DECL_ATTRIBUTES (decl)))
	{
	  if (! TREE_PUBLIC (vnode->decl))
	    warning_at (DECL_SOURCE_LOCATION (vnode->decl), OPT_Wattributes,
			"%<externally_visible%>"
			" attribute have effect only on public objects");
	  else if (vnode->finalized)
	    varpool_mark_needed_node (vnode);
	}
      if (lookup_attribute ("weakref", DECL_ATTRIBUTES (decl))
	  && vnode->finalized
	  && DECL_INITIAL (decl))
	{
	  warning_at (DECL_SOURCE_LOCATION (vnode->decl), OPT_Wattributes,
		      "%<weakref%> attribute ignored"
		      " because variable is initialized");
	  DECL_WEAK (decl) = 0;
	  DECL_ATTRIBUTES (decl) = remove_attribute ("weakref",
						      DECL_ATTRIBUTES (decl));
	}
      process_common_attributes (decl);
    }
}

/* Process CGRAPH_NODES_NEEDED queue, analyze each function (and transitively
   each reachable functions) and build cgraph.
   The function can be called multiple times after inserting new nodes
   into beginning of queue.  Just the new part of queue is re-scanned then.  */

static void
cgraph_analyze_functions (void)
{
  /* Keep track of already processed nodes when called multiple times for
     intermodule optimization.  */
  static struct cgraph_node *first_analyzed;
  struct cgraph_node *first_processed = first_analyzed;
  static struct varpool_node *first_analyzed_var;
  struct cgraph_node *node, *next;

  bitmap_obstack_initialize (NULL);
  process_function_and_variable_attributes (first_processed,
					    first_analyzed_var);
  first_processed = cgraph_nodes;
  first_analyzed_var = varpool_nodes;
  varpool_analyze_pending_decls ();
  if (cgraph_dump_file)
    {
      fprintf (cgraph_dump_file, "Initial entry points:");
      for (node = cgraph_nodes; node != first_analyzed; node = node->next)
	if (node->needed)
	  fprintf (cgraph_dump_file, " %s", cgraph_node_name (node));
      fprintf (cgraph_dump_file, "\n");
    }
  cgraph_process_new_functions ();

  /* Propagate reachability flag and lower representation of all reachable
     functions.  In the future, lowering will introduce new functions and
     new entry points on the way (by template instantiation and virtual
     method table generation for instance).  */
  while (cgraph_nodes_queue)
    {
      struct cgraph_edge *edge;
      tree decl = cgraph_nodes_queue->decl;

      node = cgraph_nodes_queue;
      cgraph_nodes_queue = cgraph_nodes_queue->next_needed;
      node->next_needed = NULL;

      /* ??? It is possible to create extern inline function and later using
	 weak alias attribute to kill its body. See
	 gcc.c-torture/compile/20011119-1.c  */
      if (!DECL_STRUCT_FUNCTION (decl))
	{
	  cgraph_reset_node (node);
	  continue;
	}

      if (!node->analyzed)
	cgraph_analyze_function (node);

      for (edge = node->callees; edge; edge = edge->next_callee)
	if (!edge->callee->reachable)
	  cgraph_mark_reachable_node (edge->callee);

      if (node->same_comdat_group)
	{
	  for (next = node->same_comdat_group;
	       next != node;
	       next = next->same_comdat_group)
	    cgraph_mark_reachable_node (next);
	}

      /* If decl is a clone of an abstract function, mark that abstract
	 function so that we don't release its body. The DECL_INITIAL() of that
         abstract function declaration will be later needed to output debug info.  */
      if (DECL_ABSTRACT_ORIGIN (decl))
	{
	  struct cgraph_node *origin_node = cgraph_node (DECL_ABSTRACT_ORIGIN (decl));
	  origin_node->abstract_and_needed = true;
	}

      /* We finalize local static variables during constructing callgraph
         edges.  Process their attributes too.  */
      process_function_and_variable_attributes (first_processed,
						first_analyzed_var);
      first_processed = cgraph_nodes;
      first_analyzed_var = varpool_nodes;
      varpool_analyze_pending_decls ();
      cgraph_process_new_functions ();
    }

  for (node = cgraph_nodes; node != first_analyzed; node = next)
    {
      tree decl = node->decl;
      next = node->next;

      if (node->local.finalized && !gimple_has_body_p (decl))
	cgraph_reset_node (node);

      if (!node->reachable && gimple_has_body_p (decl))
	{
	  if (cgraph_dump_file)
	    fprintf (cgraph_dump_file, " %s", cgraph_node_name (node));
	  cgraph_remove_node (node);
	  continue;
	}
      else
	node->next_needed = NULL;
      gcc_assert (!node->local.finalized || gimple_has_body_p (decl));
      gcc_assert (node->analyzed == node->local.finalized);
    }
    
  bitmap_obstack_release (NULL);
  first_analyzed = cgraph_nodes;
  ggc_collect ();
}


/* Analyze the whole compilation unit once it is parsed completely.  */

void
cgraph_finalize_compilation_unit (void)
{
  timevar_push (TV_CGRAPH);

  /* Do not skip analyzing the functions if there were errors, we
     miss diagnostics for following functions otherwise.  */

  /* Emit size functions we didn't inline.  */
  finalize_size_functions ();

  /* Mark alias targets necessary and emit diagnostics.  */
  finish_aliases_1 ();

  if (!quiet_flag)
    {
      fprintf (stderr, "\nAnalyzing compilation unit\n");
      fflush (stderr);
    }

  /* Gimplify and lower all functions, compute reachability and
     remove unreachable nodes.  */
  cgraph_analyze_functions ();

  /* Mark alias targets necessary and emit diagnostics.  */
  finish_aliases_1 ();

  /* Gimplify and lower thunks.  */
  cgraph_analyze_functions ();

  /* Finally drive the pass manager.  */
  cgraph_optimize ();

  timevar_pop (TV_CGRAPH);
}


/* Figure out what functions we want to assemble.  */

static void
cgraph_mark_functions_to_output (void)
{
  struct cgraph_node *node;
#ifdef ENABLE_CHECKING
  bool check_same_comdat_groups = false;

  for (node = cgraph_nodes; node; node = node->next)
    gcc_assert (!node->process);
#endif

  for (node = cgraph_nodes; node; node = node->next)
    {
      tree decl = node->decl;
      struct cgraph_edge *e;

      gcc_assert (!node->process || node->same_comdat_group);
      if (node->process)
	continue;

      for (e = node->callers; e; e = e->next_caller)
	if (e->inline_failed)
	  break;

      /* We need to output all local functions that are used and not
	 always inlined, as well as those that are reachable from
	 outside the current compilation unit.  */
      if (node->analyzed
	  && !node->global.inlined_to
	  && (!cgraph_only_called_directly_p (node)
	      || (e && node->reachable))
	  && !TREE_ASM_WRITTEN (decl)
	  && !DECL_EXTERNAL (decl))
	{
	  node->process = 1;
	  if (node->same_comdat_group)
	    {
	      struct cgraph_node *next;
	      for (next = node->same_comdat_group;
		   next != node;
		   next = next->same_comdat_group)
		next->process = 1;
	    }
	}
      else if (node->same_comdat_group)
	{
#ifdef ENABLE_CHECKING
	  check_same_comdat_groups = true;
#endif
	}
      else
	{
	  /* We should've reclaimed all functions that are not needed.  */
#ifdef ENABLE_CHECKING
	  if (!node->global.inlined_to
	      && gimple_has_body_p (decl)
	      /* FIXME: in ltrans unit when offline copy is outside partition but inline copies
		 are inside partition, we can end up not removing the body since we no longer
		 have analyzed node pointing to it.  */
	      && !node->in_other_partition
	      && !DECL_EXTERNAL (decl))
	    {
	      internal_error ("failed to reclaim unneeded function");
	    }
#endif
	  gcc_assert (node->global.inlined_to
		      || !gimple_has_body_p (decl)
		      || node->in_other_partition
		      || DECL_EXTERNAL (decl));

	}

    }
#ifdef ENABLE_CHECKING
  if (check_same_comdat_groups)
    for (node = cgraph_nodes; node; node = node->next)
      if (node->same_comdat_group && !node->process)
	{
	  tree decl = node->decl;
	  if (!node->global.inlined_to
	      && gimple_has_body_p (decl)
	      /* FIXME: in ltrans unit when offline copy is outside partition but inline copies
		 are inside partition, we can end up not removing the body since we no longer
		 have analyzed node pointing to it.  */
	      && !node->in_other_partition
	      && !DECL_EXTERNAL (decl))
	    {
	      internal_error ("failed to reclaim unneeded function");
	    }
	}
#endif
}

/* DECL is FUNCTION_DECL.  Initialize datastructures so DECL is a function
   in lowered gimple form.
   
   Set current_function_decl and cfun to newly constructed empty function body.
   return basic block in the function body.  */

static basic_block
init_lowered_empty_function (tree decl)
{
  basic_block bb;

  current_function_decl = decl;
  allocate_struct_function (decl, false);
  gimple_register_cfg_hooks ();
  init_empty_tree_cfg ();
  init_tree_ssa (cfun);
  init_ssa_operands ();
  cfun->gimple_df->in_ssa_p = true;
  DECL_INITIAL (decl) = make_node (BLOCK);

  DECL_SAVED_TREE (decl) = error_mark_node;
  cfun->curr_properties |=
    (PROP_gimple_lcf | PROP_gimple_leh | PROP_cfg | PROP_referenced_vars |
     PROP_ssa);

  /* Create BB for body of the function and connect it properly.  */
  bb = create_basic_block (NULL, (void *) 0, ENTRY_BLOCK_PTR);
  make_edge (ENTRY_BLOCK_PTR, bb, 0);
  make_edge (bb, EXIT_BLOCK_PTR, 0);

  return bb;
}

/* Adjust PTR by the constant FIXED_OFFSET, and by the vtable
   offset indicated by VIRTUAL_OFFSET, if that is
   non-null. THIS_ADJUSTING is nonzero for a this adjusting thunk and
   zero for a result adjusting thunk.  */

static tree
thunk_adjust (gimple_stmt_iterator * bsi,
	      tree ptr, bool this_adjusting,
	      HOST_WIDE_INT fixed_offset, tree virtual_offset)
{
  gimple stmt;
  tree ret;

  if (this_adjusting
      && fixed_offset != 0)
    {
      stmt = gimple_build_assign (ptr,
				  fold_build2_loc (input_location,
						   POINTER_PLUS_EXPR,
						   TREE_TYPE (ptr), ptr,
						   size_int (fixed_offset)));
      gsi_insert_after (bsi, stmt, GSI_NEW_STMT);
    }

  /* If there's a virtual offset, look up that value in the vtable and
     adjust the pointer again.  */
  if (virtual_offset)
    {
      tree vtabletmp;
      tree vtabletmp2;
      tree vtabletmp3;
      tree offsettmp;

      if (!vtable_entry_type)
	{
	  tree vfunc_type = make_node (FUNCTION_TYPE);
	  TREE_TYPE (vfunc_type) = integer_type_node;
	  TYPE_ARG_TYPES (vfunc_type) = NULL_TREE;
	  layout_type (vfunc_type);

	  vtable_entry_type = build_pointer_type (vfunc_type);
	}

      vtabletmp =
	create_tmp_var (build_pointer_type
			(build_pointer_type (vtable_entry_type)), "vptr");

      /* The vptr is always at offset zero in the object.  */
      stmt = gimple_build_assign (vtabletmp,
				  build1 (NOP_EXPR, TREE_TYPE (vtabletmp),
					  ptr));
      gsi_insert_after (bsi, stmt, GSI_NEW_STMT);
      mark_symbols_for_renaming (stmt);
      find_referenced_vars_in (stmt);

      /* Form the vtable address.  */
      vtabletmp2 = create_tmp_var (TREE_TYPE (TREE_TYPE (vtabletmp)),
				   "vtableaddr");
      stmt = gimple_build_assign (vtabletmp2,
				  build_simple_mem_ref (vtabletmp));
      gsi_insert_after (bsi, stmt, GSI_NEW_STMT);
      mark_symbols_for_renaming (stmt);
      find_referenced_vars_in (stmt);

      /* Find the entry with the vcall offset.  */
      stmt = gimple_build_assign (vtabletmp2,
				  fold_build2_loc (input_location,
						   POINTER_PLUS_EXPR,
						   TREE_TYPE (vtabletmp2),
						   vtabletmp2,
						   fold_convert (sizetype,
								 virtual_offset)));
      gsi_insert_after (bsi, stmt, GSI_NEW_STMT);

      /* Get the offset itself.  */
      vtabletmp3 = create_tmp_var (TREE_TYPE (TREE_TYPE (vtabletmp2)),
				   "vcalloffset");
      stmt = gimple_build_assign (vtabletmp3,
				  build_simple_mem_ref (vtabletmp2));
      gsi_insert_after (bsi, stmt, GSI_NEW_STMT);
      mark_symbols_for_renaming (stmt);
      find_referenced_vars_in (stmt);

      /* Cast to sizetype.  */
      offsettmp = create_tmp_var (sizetype, "offset");
      stmt = gimple_build_assign (offsettmp, fold_convert (sizetype, vtabletmp3));
      gsi_insert_after (bsi, stmt, GSI_NEW_STMT);
      mark_symbols_for_renaming (stmt);
      find_referenced_vars_in (stmt);

      /* Adjust the `this' pointer.  */
      ptr = fold_build2_loc (input_location,
			     POINTER_PLUS_EXPR, TREE_TYPE (ptr), ptr,
			     offsettmp);
    }

  if (!this_adjusting
      && fixed_offset != 0)
    /* Adjust the pointer by the constant.  */
    {
      tree ptrtmp;

      if (TREE_CODE (ptr) == VAR_DECL)
        ptrtmp = ptr;
      else
        {
          ptrtmp = create_tmp_var (TREE_TYPE (ptr), "ptr");
          stmt = gimple_build_assign (ptrtmp, ptr);
	  gsi_insert_after (bsi, stmt, GSI_NEW_STMT);
	  mark_symbols_for_renaming (stmt);
	  find_referenced_vars_in (stmt);
	}
      ptr = fold_build2_loc (input_location,
			     POINTER_PLUS_EXPR, TREE_TYPE (ptrtmp), ptrtmp,
			     size_int (fixed_offset));
    }

  /* Emit the statement and gimplify the adjustment expression.  */
  ret = create_tmp_var (TREE_TYPE (ptr), "adjusted_this");
  stmt = gimple_build_assign (ret, ptr);
  mark_symbols_for_renaming (stmt);
  find_referenced_vars_in (stmt);
  gsi_insert_after (bsi, stmt, GSI_NEW_STMT);

  return ret;
}

/* Produce assembler for thunk NODE.  */

static void
assemble_thunk (struct cgraph_node *node)
{
  bool this_adjusting = node->thunk.this_adjusting;
  HOST_WIDE_INT fixed_offset = node->thunk.fixed_offset;
  HOST_WIDE_INT virtual_value = node->thunk.virtual_value;
  tree virtual_offset = NULL;
  tree alias = node->thunk.alias;
  tree thunk_fndecl = node->decl;
  tree a = DECL_ARGUMENTS (thunk_fndecl);

  current_function_decl = thunk_fndecl;

  /* Ensure thunks are emitted in their correct sections.  */
  resolve_unique_section (thunk_fndecl, 0, flag_function_sections);

  if (this_adjusting
      && targetm.asm_out.can_output_mi_thunk (thunk_fndecl, fixed_offset,
					      virtual_value, alias))
    {
      const char *fnname;
      tree fn_block;
      
      DECL_RESULT (thunk_fndecl)
	= build_decl (DECL_SOURCE_LOCATION (thunk_fndecl),
		      RESULT_DECL, 0, integer_type_node);
      fnname = IDENTIFIER_POINTER (DECL_ASSEMBLER_NAME (thunk_fndecl));

      /* The back end expects DECL_INITIAL to contain a BLOCK, so we
	 create one.  */
      fn_block = make_node (BLOCK);
      BLOCK_VARS (fn_block) = a;
      DECL_INITIAL (thunk_fndecl) = fn_block;
      init_function_start (thunk_fndecl);
      cfun->is_thunk = 1;
      assemble_start_function (thunk_fndecl, fnname);

      targetm.asm_out.output_mi_thunk (asm_out_file, thunk_fndecl,
				       fixed_offset, virtual_value, alias);

      assemble_end_function (thunk_fndecl, fnname);
      init_insn_lengths ();
      free_after_compilation (cfun);
      set_cfun (NULL);
      TREE_ASM_WRITTEN (thunk_fndecl) = 1;
    }
  else
    {
      tree restype;
      basic_block bb, then_bb, else_bb, return_bb;
      gimple_stmt_iterator bsi;
      int nargs = 0;
      tree arg;
      int i;
      tree resdecl;
      tree restmp = NULL;
      VEC(tree, heap) *vargs;

      gimple call;
      gimple ret;

      DECL_IGNORED_P (thunk_fndecl) = 1;
      bitmap_obstack_initialize (NULL);

      if (node->thunk.virtual_offset_p)
        virtual_offset = size_int (virtual_value);

      /* Build the return declaration for the function.  */
      restype = TREE_TYPE (TREE_TYPE (thunk_fndecl));
      if (DECL_RESULT (thunk_fndecl) == NULL_TREE)
	{
	  resdecl = build_decl (input_location, RESULT_DECL, 0, restype);
	  DECL_ARTIFICIAL (resdecl) = 1;
	  DECL_IGNORED_P (resdecl) = 1;
	  DECL_RESULT (thunk_fndecl) = resdecl;
	}
      else
	resdecl = DECL_RESULT (thunk_fndecl);

      bb = then_bb = else_bb = return_bb = init_lowered_empty_function (thunk_fndecl);

      bsi = gsi_start_bb (bb);

      /* Build call to the function being thunked.  */
      if (!VOID_TYPE_P (restype))
	{
	  if (!is_gimple_reg_type (restype))
	    {
	      restmp = resdecl;
	      add_local_decl (cfun, restmp);
	      BLOCK_VARS (DECL_INITIAL (current_function_decl)) = restmp;
	    }
	  else
            restmp = create_tmp_var_raw (restype, "retval");
	}

      for (arg = a; arg; arg = DECL_CHAIN (arg))
        nargs++;
      vargs = VEC_alloc (tree, heap, nargs);
      if (this_adjusting)
        VEC_quick_push (tree, vargs,
			thunk_adjust (&bsi,
				      a, 1, fixed_offset,
				      virtual_offset));
      else
        VEC_quick_push (tree, vargs, a);
      for (i = 1, arg = DECL_CHAIN (a); i < nargs; i++, arg = DECL_CHAIN (arg))
        VEC_quick_push (tree, vargs, arg);
      call = gimple_build_call_vec (build_fold_addr_expr_loc (0, alias), vargs);
      VEC_free (tree, heap, vargs);
      gimple_call_set_cannot_inline (call, true);
      gimple_call_set_from_thunk (call, true);
      if (restmp)
        gimple_call_set_lhs (call, restmp);
      gsi_insert_after (&bsi, call, GSI_NEW_STMT);
      mark_symbols_for_renaming (call);
      find_referenced_vars_in (call);
      update_stmt (call);

      if (restmp && !this_adjusting)
        {
	  tree true_label = NULL_TREE;

	  if (TREE_CODE (TREE_TYPE (restmp)) == POINTER_TYPE)
	    {
	      gimple stmt;
	      /* If the return type is a pointer, we need to
		 protect against NULL.  We know there will be an
		 adjustment, because that's why we're emitting a
		 thunk.  */
	      then_bb = create_basic_block (NULL, (void *) 0, bb);
	      return_bb = create_basic_block (NULL, (void *) 0, then_bb);
	      else_bb = create_basic_block (NULL, (void *) 0, else_bb);
	      remove_edge (single_succ_edge (bb));
	      true_label = gimple_block_label (then_bb);
	      stmt = gimple_build_cond (NE_EXPR, restmp,
	      				build_zero_cst (TREE_TYPE (restmp)),
	      			        NULL_TREE, NULL_TREE);
	      gsi_insert_after (&bsi, stmt, GSI_NEW_STMT);
	      make_edge (bb, then_bb, EDGE_TRUE_VALUE);
	      make_edge (bb, else_bb, EDGE_FALSE_VALUE);
	      make_edge (return_bb, EXIT_BLOCK_PTR, 0);
	      make_edge (then_bb, return_bb, EDGE_FALLTHRU);
	      make_edge (else_bb, return_bb, EDGE_FALLTHRU);
	      bsi = gsi_last_bb (then_bb);
	    }

	  restmp = thunk_adjust (&bsi, restmp, /*this_adjusting=*/0,
			         fixed_offset, virtual_offset);
	  if (true_label)
	    {
	      gimple stmt;
	      bsi = gsi_last_bb (else_bb);
	      stmt = gimple_build_assign (restmp,
					  build_zero_cst (TREE_TYPE (restmp)));
	      gsi_insert_after (&bsi, stmt, GSI_NEW_STMT);
	      bsi = gsi_last_bb (return_bb);
	    }
	}
      else
        gimple_call_set_tail (call, true);

      /* Build return value.  */
      ret = gimple_build_return (restmp);
      gsi_insert_after (&bsi, ret, GSI_NEW_STMT);

      delete_unreachable_blocks ();
      update_ssa (TODO_update_ssa);

      cgraph_remove_same_body_alias (node);
      /* Since we want to emit the thunk, we explicitly mark its name as
	 referenced.  */
      cgraph_add_new_function (thunk_fndecl, true);
      bitmap_obstack_release (NULL);
    }
  current_function_decl = NULL;
}

/* Expand function specified by NODE.  */

static void
cgraph_expand_function (struct cgraph_node *node)
{
  tree decl = node->decl;

  /* We ought to not compile any inline clones.  */
  gcc_assert (!node->global.inlined_to);

  announce_function (decl);
  node->process = 0;
  if (node->same_body)
    {
      struct cgraph_node *alias, *next;
      bool saved_alias = node->alias;
      for (alias = node->same_body;
      	   alias && alias->next; alias = alias->next)
        ;
      /* Walk aliases in the order they were created; it is possible that
         thunks refers to the aliases made earlier.  */
      for (; alias; alias = next)
        {
	  next = alias->previous;
	  if (!alias->thunk.thunk_p)
	    assemble_alias (alias->decl,
			    DECL_ASSEMBLER_NAME (alias->thunk.alias));
	  else
	    assemble_thunk (alias);
	}
      node->alias = saved_alias;
      cgraph_process_new_functions ();
    }

  gcc_assert (node->lowered);

  /* Generate RTL for the body of DECL.  */
  tree_rest_of_compilation (decl);

  /* Make sure that BE didn't give up on compiling.  */
  gcc_assert (TREE_ASM_WRITTEN (decl));
  current_function_decl = NULL;
  gcc_assert (!cgraph_preserve_function_body_p (decl));
  cgraph_release_function_body (node);
  /* Eliminate all call edges.  This is important so the GIMPLE_CALL no longer
     points to the dead function body.  */
  cgraph_node_remove_callees (node);

  cgraph_function_flags_ready = true;
}

/* Return true when CALLER_DECL should be inlined into CALLEE_DECL.  */

bool
cgraph_inline_p (struct cgraph_edge *e, cgraph_inline_failed_t *reason)
{
  *reason = e->inline_failed;
  return !e->inline_failed;
}



/* Expand all functions that must be output.

   Attempt to topologically sort the nodes so function is output when
   all called functions are already assembled to allow data to be
   propagated across the callgraph.  Use a stack to get smaller distance
   between a function and its callees (later we may choose to use a more
   sophisticated algorithm for function reordering; we will likely want
   to use subsections to make the output functions appear in top-down
   order).  */

static void
cgraph_expand_all_functions (void)
{
  struct cgraph_node *node;
  struct cgraph_node **order = XCNEWVEC (struct cgraph_node *, cgraph_n_nodes);
  int order_pos, new_order_pos = 0;
  int i;

  order_pos = cgraph_postorder (order);
  gcc_assert (order_pos == cgraph_n_nodes);

  /* Garbage collector may remove inline clones we eliminate during
     optimization.  So we must be sure to not reference them.  */
  for (i = 0; i < order_pos; i++)
    if (order[i]->process)
      order[new_order_pos++] = order[i];

  for (i = new_order_pos - 1; i >= 0; i--)
    {
      node = order[i];
      if (node->process)
	{
	  gcc_assert (node->reachable);
	  node->process = 0;
	  cgraph_expand_function (node);
	}
    }
  cgraph_process_new_functions ();

  free (order);

}

/* This is used to sort the node types by the cgraph order number.  */

enum cgraph_order_sort_kind
{
  ORDER_UNDEFINED = 0,
  ORDER_FUNCTION,
  ORDER_VAR,
  ORDER_ASM
};

struct cgraph_order_sort
{
  enum cgraph_order_sort_kind kind;
  union
  {
    struct cgraph_node *f;
    struct varpool_node *v;
    struct cgraph_asm_node *a;
  } u;
};

/* Output all functions, variables, and asm statements in the order
   according to their order fields, which is the order in which they
   appeared in the file.  This implements -fno-toplevel-reorder.  In
   this mode we may output functions and variables which don't really
   need to be output.  */

static void
cgraph_output_in_order (void)
{
  int max;
  struct cgraph_order_sort *nodes;
  int i;
  struct cgraph_node *pf;
  struct varpool_node *pv;
  struct cgraph_asm_node *pa;

  max = cgraph_order;
  nodes = XCNEWVEC (struct cgraph_order_sort, max);

  varpool_analyze_pending_decls ();

  for (pf = cgraph_nodes; pf; pf = pf->next)
    {
      if (pf->process)
	{
	  i = pf->order;
	  gcc_assert (nodes[i].kind == ORDER_UNDEFINED);
	  nodes[i].kind = ORDER_FUNCTION;
	  nodes[i].u.f = pf;
	}
    }

  for (pv = varpool_nodes_queue; pv; pv = pv->next_needed)
    {
      i = pv->order;
      gcc_assert (nodes[i].kind == ORDER_UNDEFINED);
      nodes[i].kind = ORDER_VAR;
      nodes[i].u.v = pv;
    }

  for (pa = cgraph_asm_nodes; pa; pa = pa->next)
    {
      i = pa->order;
      gcc_assert (nodes[i].kind == ORDER_UNDEFINED);
      nodes[i].kind = ORDER_ASM;
      nodes[i].u.a = pa;
    }

  /* In toplevel reorder mode we output all statics; mark them as needed.  */
  for (i = 0; i < max; ++i)
    {
      if (nodes[i].kind == ORDER_VAR)
        {
	  varpool_mark_needed_node (nodes[i].u.v);
	}
    }
  varpool_empty_needed_queue ();

  for (i = 0; i < max; ++i)
    if (nodes[i].kind == ORDER_VAR)
      varpool_finalize_named_section_flags (nodes[i].u.v);

  for (i = 0; i < max; ++i)
    {
      switch (nodes[i].kind)
	{
	case ORDER_FUNCTION:
	  nodes[i].u.f->process = 0;
	  cgraph_expand_function (nodes[i].u.f);
	  break;

	case ORDER_VAR:
	  varpool_assemble_decl (nodes[i].u.v);
	  break;

	case ORDER_ASM:
	  assemble_asm (nodes[i].u.a->asm_str);
	  break;

	case ORDER_UNDEFINED:
	  break;

	default:
	  gcc_unreachable ();
	}
    }

  cgraph_asm_nodes = NULL;
  free (nodes);
}

/* Return true when function body of DECL still needs to be kept around
   for later re-use.  */
bool
cgraph_preserve_function_body_p (tree decl)
{
  struct cgraph_node *node;

  gcc_assert (cgraph_global_info_ready);
  /* Look if there is any clone around.  */
  node = cgraph_node (decl);
  if (node->clones)
    return true;
  return false;
}

static void
ipa_passes (void)
{
  set_cfun (NULL);
  current_function_decl = NULL;
  gimple_register_cfg_hooks ();
  bitmap_obstack_initialize (NULL);

  invoke_plugin_callbacks (PLUGIN_ALL_IPA_PASSES_START, NULL);

  if (!in_lto_p)
    {
      execute_ipa_pass_list (all_small_ipa_passes);
      if (seen_error ())
	return;
    }

  /* If pass_all_early_optimizations was not scheduled, the state of
     the cgraph will not be properly updated.  Update it now.  */
  if (cgraph_state < CGRAPH_STATE_IPA_SSA)
    cgraph_state = CGRAPH_STATE_IPA_SSA;

  if (!in_lto_p)
    {
      /* Process new functions added.  */
      set_cfun (NULL);
      current_function_decl = NULL;
      cgraph_process_new_functions ();
    }

  bitmap_obstack_release (NULL);
}


/* Perform simple optimizations based on callgraph.  */

void
cgraph_optimize (void)
{
  if (seen_error ())
    return;

  /* Frontend may output common variables after the unit has been finalized.
     It is safe to deal with them here as they are always zero initialized.  */
  varpool_analyze_pending_decls ();

  timevar_push (TV_CGRAPHOPT);
  if (pre_ipa_mem_report)
    {
      fprintf (stderr, "Memory consumption before IPA\n");
      dump_memory_report (false);
    }
  if (!quiet_flag)
    fprintf (stderr, "Performing interprocedural optimizations\n");
  cgraph_state = CGRAPH_STATE_IPA;

  /* Don't run the IPA passes if there was any error or sorry messages.  */
  if (!seen_error ())
    ipa_passes ();

  /* Do nothing else if any IPA pass found errors.  */
  if (seen_error ())
    {
      timevar_pop (TV_CGRAPHOPT);
      return;
    }

  /* This pass remove bodies of extern inline functions we never inlined.
     Do this later so other IPA passes see what is really going on.  */
  cgraph_remove_unreachable_nodes (false, dump_file);
  cgraph_global_info_ready = true;
  if (post_ipa_mem_report)
    {
      fprintf (stderr, "Memory consumption after IPA\n");
      dump_memory_report (false);
    }
  timevar_pop (TV_CGRAPHOPT);

  /* Output everything.  */
  (*debug_hooks->assembly_start) ();
  if (!quiet_flag)
    fprintf (stderr, "Assembling functions:\n");

  cgraph_materialize_all_clones ();
  cgraph_mark_functions_to_output ();

  cgraph_state = CGRAPH_STATE_EXPANSION;
  if (!flag_toplevel_reorder)
    cgraph_output_in_order ();
  else
    {
      cgraph_output_pending_asms ();

      cgraph_expand_all_functions ();
      varpool_remove_unreferenced_decls ();

      varpool_assemble_pending_decls ();
    }
  cgraph_process_new_functions ();
  cgraph_state = CGRAPH_STATE_FINISHED;

#ifdef ENABLE_CHECKING
  /* Double check that all inline clones are gone and that all
     function bodies have been released from memory.  */
  if (!seen_error ())
    {
      struct cgraph_node *node;
      bool error_found = false;

      for (node = cgraph_nodes; node; node = node->next)
	if (node->analyzed
	    && (node->global.inlined_to
		|| gimple_has_body_p (node->decl)))
	  {
	    error_found = true;
	  }
      if (error_found)
	internal_error ("nodes with unreleased memory found");
    }
#endif
}

void
init_cgraph (void)
{
  if (!cgraph_dump_file)
    cgraph_dump_file = dump_begin (TDI_cgraph, NULL);
}

/* Given virtual clone, turn it into actual clone.  */
static void
cgraph_materialize_clone (struct cgraph_node *node)
{
  gcc_unreachable ();
}

/* If necessary, change the function declaration in the call statement
   associated with E so that it corresponds to the edge callee.  */

gimple
cgraph_redirect_edge_call_stmt_to_callee (struct cgraph_edge *e)
{
  tree decl = gimple_call_fndecl (e->call_stmt);
  gimple new_stmt;
  gimple_stmt_iterator gsi;
  bool gsi_computed = false;
#ifdef ENABLE_CHECKING
  struct cgraph_node *node;
#endif

  if (e->indirect_unknown_callee
      || decl == e->callee->decl
      /* Don't update call from same body alias to the real function.  */
      || (decl && cgraph_get_node (decl) == cgraph_get_node (e->callee->decl)))
    return e->call_stmt;

#ifdef ENABLE_CHECKING
  if (decl)
    {
      node = cgraph_get_node (decl);
      gcc_assert (!node || !node->clone.combined_args_to_skip);
    }
#endif

  if (cgraph_dump_file)
    {
      fprintf (cgraph_dump_file, "updating call of %s/%i -> %s/%i: ",
	       cgraph_node_name (e->caller), e->caller->uid,
	       cgraph_node_name (e->callee), e->callee->uid);
      print_gimple_stmt (cgraph_dump_file, e->call_stmt, 0, dump_flags);
      if (e->callee->clone.combined_args_to_skip)
	{
	  fprintf (cgraph_dump_file, " combined args to skip: ");
	  dump_bitmap (cgraph_dump_file,
		       e->callee->clone.combined_args_to_skip);
	}
    }

  if (e->indirect_info &&
      e->indirect_info->thunk_delta != 0
      && (!e->callee->clone.combined_args_to_skip
	  || !bitmap_bit_p (e->callee->clone.combined_args_to_skip, 0)))
    {
      if (cgraph_dump_file)
	fprintf (cgraph_dump_file, "          Thunk delta is "
		 HOST_WIDE_INT_PRINT_DEC "\n", e->indirect_info->thunk_delta);
      gsi = gsi_for_stmt (e->call_stmt);
      gsi_computed = true;
      gimple_adjust_this_by_delta (&gsi,
				   build_int_cst (sizetype,
					       e->indirect_info->thunk_delta));
      e->indirect_info->thunk_delta = 0;
    }

  if (e->callee->clone.combined_args_to_skip)
    {
      int lp_nr;

      new_stmt
	= gimple_call_copy_skip_args (e->call_stmt,
				      e->callee->clone.combined_args_to_skip);
      gimple_call_set_fndecl (new_stmt, e->callee->decl);

      if (gimple_vdef (new_stmt)
	  && TREE_CODE (gimple_vdef (new_stmt)) == SSA_NAME)
	SSA_NAME_DEF_STMT (gimple_vdef (new_stmt)) = new_stmt;

      if (!gsi_computed)
	gsi = gsi_for_stmt (e->call_stmt);
      gsi_replace (&gsi, new_stmt, false);
      /* We need to defer cleaning EH info on the new statement to
         fixup-cfg.  We may not have dominator information at this point
	 and thus would end up with unreachable blocks and have no way
	 to communicate that we need to run CFG cleanup then.  */
      lp_nr = lookup_stmt_eh_lp (e->call_stmt);
      if (lp_nr != 0)
	{
	  remove_stmt_from_eh_lp (e->call_stmt);
	  add_stmt_to_eh_lp (new_stmt, lp_nr);
	}
    }
  else
    {
      new_stmt = e->call_stmt;
      gimple_call_set_fndecl (new_stmt, e->callee->decl);
      update_stmt (new_stmt);
    }

  cgraph_set_call_stmt_including_clones (e->caller, e->call_stmt, new_stmt);

  if (cgraph_dump_file)
    {
      fprintf (cgraph_dump_file, "  updated to:");
      print_gimple_stmt (cgraph_dump_file, e->call_stmt, 0, dump_flags);
    }
  return new_stmt;
}

/* Once all functions from compilation unit are in memory, produce all clones
   and update all calls.  We might also do this on demand if we don't want to
   bring all functions to memory prior compilation, but current WHOPR
   implementation does that and it is is bit easier to keep everything right in
   this order.  */
void
cgraph_materialize_all_clones (void)
{
  struct cgraph_node *node;
  bool stabilized = false;

  if (cgraph_dump_file)
    fprintf (cgraph_dump_file, "Materializing clones\n");

  /* We can also do topological order, but number of iterations should be
     bounded by number of IPA passes since single IPA pass is probably not
     going to create clones of clones it created itself.  */
  while (!stabilized)
    {
      stabilized = true;
      for (node = cgraph_nodes; node; node = node->next)
        {
	  if (node->clone_of && node->decl != node->clone_of->decl
	      && !gimple_has_body_p (node->decl))
	    {
	      if (gimple_has_body_p (node->clone_of->decl))
	        {
		  if (cgraph_dump_file)
		    {
		      fprintf (cgraph_dump_file, "cloning %s to %s\n",
			       cgraph_node_name (node->clone_of),
			       cgraph_node_name (node));
		      if (node->clone.tree_map)
		        {
			  unsigned int i;
		          fprintf (cgraph_dump_file, "   replace map: ");
			  for (i = 0; i < VEC_length (ipa_replace_map_p,
			  			      node->clone.tree_map);
						      i++)
			    {
			      struct ipa_replace_map *replace_info;
			      replace_info = VEC_index (ipa_replace_map_p,
			      				node->clone.tree_map,
							i);
			      print_generic_expr (cgraph_dump_file, replace_info->old_tree, 0);
			      fprintf (cgraph_dump_file, " -> ");
			      print_generic_expr (cgraph_dump_file, replace_info->new_tree, 0);
			      fprintf (cgraph_dump_file, "%s%s;",
			      	       replace_info->replace_p ? "(replace)":"",
				       replace_info->ref_p ? "(ref)":"");
			    }
			  fprintf (cgraph_dump_file, "\n");
			}
		      if (node->clone.args_to_skip)
			{
		          fprintf (cgraph_dump_file, "   args_to_skip: ");
		          dump_bitmap (cgraph_dump_file, node->clone.args_to_skip);
			}
		      if (node->clone.args_to_skip)
			{
		          fprintf (cgraph_dump_file, "   combined_args_to_skip:");
		          dump_bitmap (cgraph_dump_file, node->clone.combined_args_to_skip);
			}
		    }
		  cgraph_materialize_clone (node);
		  stabilized = false;
	        }
	    }
	}
    }
  for (node = cgraph_nodes; node; node = node->next)
    if (!node->analyzed && node->callees)
      cgraph_node_remove_callees (node);
  if (cgraph_dump_file)
    fprintf (cgraph_dump_file, "Materialization Call site updates done.\n");
  cgraph_remove_unreachable_nodes (false, cgraph_dump_file);
}

EXTERN_C_END

#include "gt-cgraphunit.h"
