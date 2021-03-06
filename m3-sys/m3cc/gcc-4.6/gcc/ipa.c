/* Modula-3: modified */

/* Basic IPA optimizations and utilities.
   Copyright (C) 2003, 2004, 2005, 2007, 2008, 2009, 2010, 2011
   Free Software Foundation, Inc.

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

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "cgraph.h"
#include "tree-pass.h"
#include "timevar.h"
#include "gimple.h"
#include "ggc.h"
#include "flags.h"
#include "pointer-set.h"
#include "target.h"
#include "tree-iterator.h"

EXTERN_C_START

/* Fill array order with all nodes with output flag set in the reverse
   topological order.  */

int
cgraph_postorder (struct cgraph_node **order)
{
  struct cgraph_node *node, *node2;
  int stack_size = 0;
  int order_pos = 0;
  struct cgraph_edge *edge, last;
  int pass;

  struct cgraph_node **stack =
    XCNEWVEC (struct cgraph_node *, cgraph_n_nodes);

  /* We have to deal with cycles nicely, so use a depth first traversal
     output algorithm.  Ignore the fact that some functions won't need
     to be output and put them into order as well, so we get dependencies
     right through inline functions.  */
  for (node = cgraph_nodes; node; node = node->next)
    node->aux = NULL;
  for (pass = 0; pass < 2; pass++)
    for (node = cgraph_nodes; node; node = node->next)
      if (!node->aux
	  && (pass
	      || (!node->address_taken
		  && !node->global.inlined_to
		  && !cgraph_only_called_directly_p (node))))
	{
	  node2 = node;
	  if (!node->callers)
	    node->aux = &last;
	  else
	    node->aux = node->callers;
	  while (node2)
	    {
	      while (node2->aux != &last)
		{
		  edge = (struct cgraph_edge *) node2->aux;
		  if (edge->next_caller)
		    node2->aux = edge->next_caller;
		  else
		    node2->aux = &last;
		  if (!edge->caller->aux)
		    {
		      if (!edge->caller->callers)
			edge->caller->aux = &last;
		      else
			edge->caller->aux = edge->caller->callers;
		      stack[stack_size++] = node2;
		      node2 = edge->caller;
		      break;
		    }
		}
	      if (node2->aux == &last)
		{
		  order[order_pos++] = node2;
		  if (stack_size)
		    node2 = stack[--stack_size];
		  else
		    node2 = NULL;
		}
	    }
	}
  free (stack);
  for (node = cgraph_nodes; node; node = node->next)
    node->aux = NULL;
  return order_pos;
}

/* Look for all functions inlined to NODE and update their inlined_to pointers
   to INLINED_TO.  */

static void
update_inlined_to_pointer (struct cgraph_node *node, struct cgraph_node *inlined_to)
{
  struct cgraph_edge *e;
  for (e = node->callees; e; e = e->next_callee)
    if (e->callee->global.inlined_to)
      {
        e->callee->global.inlined_to = inlined_to;
	update_inlined_to_pointer (e->callee, inlined_to);
      }
}

/* Add cgraph NODE to queue starting at FIRST.

   The queue is linked via AUX pointers and terminated by pointer to 1.
   We enqueue nodes at two occasions: when we find them reachable or when we find
   their bodies needed for further clonning.  In the second case we mark them
   by pointer to 2 after processing so they are re-queue when they become
   reachable.  */

static void
enqueue_cgraph_node (struct cgraph_node *node, struct cgraph_node **first)
{
  /* Node is still in queue; do nothing.  */
  if (node->aux && node->aux != (void *) 2)
    return;
  /* Node was already processed as unreachable, re-enqueue
     only if it became reachable now.  */
  if (node->aux == (void *)2 && !node->reachable)
    return;
  node->aux = *first;
  *first = node;
}

/* Add varpool NODE to queue starting at FIRST.  */

static void
enqueue_varpool_node (struct varpool_node *node, struct varpool_node **first)
{
  node->aux = *first;
  *first = node;
}

/* Process references.  */

static void
process_references (struct ipa_ref_list *list,
		    struct cgraph_node **first,
		    struct varpool_node **first_varpool,
		    bool before_inlining_p)
{
  int i;
  struct ipa_ref *ref;
  for (i = 0; ipa_ref_list_reference_iterate (list, i, ref); i++)
    {
      if (ref->refered_type == IPA_REF_CGRAPH)
	{
	  struct cgraph_node *node = ipa_ref_node (ref);
	  if (!node->reachable
	      && node->analyzed
	      && (!DECL_EXTERNAL (node->decl)
	          || before_inlining_p))
	    node->reachable = true;
	  enqueue_cgraph_node (node, first);
	}
      else
	{
	  struct varpool_node *node = ipa_ref_varpool_node (ref);
	  if (!node->needed)
	    {
	      varpool_mark_needed_node (node);
	      enqueue_varpool_node (node, first_varpool);
	    }
	}
    }
}

/* Return true when function can be marked local.  */

static bool
cgraph_local_node_p (struct cgraph_node *node)
{
   return (cgraph_only_called_directly_p (node)
	   && node->analyzed
	   && !DECL_EXTERNAL (node->decl)
	   && !node->local.externally_visible
	   && !node->reachable_from_other_partition
	   && !node->in_other_partition);
}

/* Perform reachability analysis and reclaim all unreachable nodes.
   If BEFORE_INLINING_P is true this function is called before inlining
   decisions has been made.  If BEFORE_INLINING_P is false this function also
   removes unneeded bodies of extern inline functions.  */

bool
cgraph_remove_unreachable_nodes (bool before_inlining_p, FILE *file)
{
  struct cgraph_node *first = (struct cgraph_node *) (void *) 1;
  struct varpool_node *first_varpool = (struct varpool_node *) (void *) 1;
  struct cgraph_node *node, *next;
  struct varpool_node *vnode, *vnext;
  bool changed = false;

  if (file)
    fprintf (file, "\nReclaiming functions:");
#ifdef ENABLE_CHECKING
  for (node = cgraph_nodes; node; node = node->next)
    gcc_assert (!node->aux);
  for (vnode = varpool_nodes; vnode; vnode = vnode->next)
    gcc_assert (!vnode->aux);
#endif
  varpool_reset_queue ();
  /* Mark functions whose bodies are obviously needed.
     This is mostly when they can be referenced externally.  Inline clones
     are special since their declarations are shared with master clone and thus
     cgraph_can_remove_if_no_direct_calls_and_refs_p should not be called on them.  */
  for (node = cgraph_nodes; node; node = node->next)
    if (node->analyzed && !node->global.inlined_to
	&& (!cgraph_can_remove_if_no_direct_calls_and_refs_p (node)
	    /* Keep around virtual functions for possible devirtualization.  */
	    || (before_inlining_p
		&& DECL_VIRTUAL_P (node->decl)
		&& (DECL_COMDAT (node->decl) || DECL_EXTERNAL (node->decl)))
	    /* Also external functions with address taken are better to stay
	       for indirect inlining.  */
	    || (before_inlining_p
		&& DECL_EXTERNAL (node->decl)
		&& node->address_taken)))
      {
        gcc_assert (!node->global.inlined_to);
	enqueue_cgraph_node (node, &first);
	node->reachable = true;
      }
    else
      {
        gcc_assert (!node->aux);
	node->reachable = false;
      }

  /* Mark variables that are obviously needed.  */
  for (vnode = varpool_nodes; vnode; vnode = vnode->next)
    {
      vnode->next_needed = NULL;
      vnode->prev_needed = NULL;
      if ((vnode->analyzed || vnode->force_output)
	  && !varpool_can_remove_if_no_refs (vnode))
	{
	  vnode->needed = false;
	  varpool_mark_needed_node (vnode);
	  enqueue_varpool_node (vnode, &first_varpool);
	}
      else
	vnode->needed = false;
    }

  /* Perform reachability analysis.  As a special case do not consider
     extern inline functions not inlined as live because we won't output
     them at all. 

     We maintain two worklist, one for cgraph nodes other for varpools and
     are finished once both are empty.  */

  while (first != (struct cgraph_node *) (void *) 1
  	 || first_varpool != (struct varpool_node *) (void *) 1)
    {
      if (first != (struct cgraph_node *) (void *) 1)
	{
	  struct cgraph_edge *e;
	  node = first;
	  first = (struct cgraph_node *) first->aux;
	  if (!node->reachable)
	    node->aux = (void *)2;

	  /* If we found this node reachable, first mark on the callees
	     reachable too, unless they are direct calls to extern inline functions
	     we decided to not inline.  */
	  if (node->reachable)
	    {
	      for (e = node->callees; e; e = e->next_callee)
		{
		  if (!e->callee->reachable
		      && node->analyzed
		      && (!e->inline_failed
			  || !DECL_EXTERNAL (e->callee->decl)
			  || before_inlining_p))
		    e->callee->reachable = true;
		  enqueue_cgraph_node (e->callee, &first);
		}
	      process_references (&node->ref_list, &first, &first_varpool, before_inlining_p);
	    }

	  /* If any function in a comdat group is reachable, force
	     all other functions in the same comdat group to be
	     also reachable.  */
	  if (node->same_comdat_group
	      && node->reachable
	      && !node->global.inlined_to)
	    {
	      for (next = node->same_comdat_group;
		   next != node;
		   next = next->same_comdat_group)
		if (!next->reachable)
		  {
		    next->reachable = true;
		    enqueue_cgraph_node (next, &first);
		  }
	    }

	  /* We can freely remove inline clones even if they are cloned, however if
	     function is clone of real clone, we must keep it around in order to
	     make materialize_clones produce function body with the changes
	     applied.  */
	  while (node->clone_of && !node->clone_of->aux
	         && !gimple_has_body_p (node->decl))
	    {
	      bool noninline = node->clone_of->decl != node->decl;
	      node = node->clone_of;
	      if (noninline && !node->reachable && !node->aux)
	      	{
		  enqueue_cgraph_node (node, &first);
		  break;
		}
	    }
	}
      if (first_varpool != (struct varpool_node *) (void *) 1)
	{
	  vnode = first_varpool;
	  first_varpool = (struct varpool_node *)first_varpool->aux;
	  vnode->aux = NULL;
	  process_references (&vnode->ref_list, &first, &first_varpool, before_inlining_p);
	  /* If any function in a comdat group is reachable, force
	     all other functions in the same comdat group to be
	     also reachable.  */
	  if (vnode->same_comdat_group)
	    {
	      struct varpool_node *next;
	      for (next = vnode->same_comdat_group;
		   next != vnode;
		   next = next->same_comdat_group)
		if (!next->needed)
		  {
		    varpool_mark_needed_node (next);
		    enqueue_varpool_node (next, &first_varpool);
		  }
	    }
	}
    }

  /* Remove unreachable nodes. 

     Completely unreachable functions can be fully removed from the callgraph.
     Extern inline functions that we decided to not inline need to become unanalyzed nodes of
     callgraph (so we still have edges to them).  We remove function body then.

     Also we need to care functions that are unreachable but we need to keep them around
     for later clonning.  In this case we also turn them to unanalyzed nodes, but
     keep the body around.  */
  for (node = cgraph_nodes; node; node = next)
    {
      next = node->next;
      if (node->aux && !node->reachable)
        {
	  cgraph_node_remove_callees (node);
	  node->analyzed = false;
	  node->local.inlinable = false;
	}
      if (!node->aux)
	{
	  struct cgraph_edge *e;
	  bool found = false;
	  int i;
	  struct ipa_ref *ref;

          node->global.inlined_to = NULL;
	  if (file)
	    fprintf (file, " %s", cgraph_node_name (node));
	  /* See if there is reachable caller.  */
	  for (e = node->callers; e && !found; e = e->next_caller)
	    if (e->caller->reachable)
	      found = true;
	  for (i = 0; (ipa_ref_list_refering_iterate (&node->ref_list, i, ref)
		       && !found); i++)
	    if (ref->refering_type == IPA_REF_CGRAPH
		&& ipa_ref_refering_node (ref)->reachable)
	      found = true;
	    else if (ref->refering_type == IPA_REF_VARPOOL
		     && ipa_ref_refering_varpool_node (ref)->needed)
	      found = true;

	  /* If so, we need to keep node in the callgraph.  */
	  if (found)
	    {
	      if (node->analyzed)
		{
		  struct cgraph_node *clone;

		  /* If there are still clones, we must keep body around.
		     Otherwise we can just remove the body but keep the clone.  */
		  for (clone = node->clones; clone;
		       clone = clone->next_sibling_clone)
		    if (clone->aux)
		      break;
		  if (!clone)
		    {
		      cgraph_release_function_body (node);
		      node->local.inlinable = false;
		      if (node->prev_sibling_clone)
			node->prev_sibling_clone->next_sibling_clone = node->next_sibling_clone;
		      else if (node->clone_of)
			node->clone_of->clones = node->next_sibling_clone;
		      if (node->next_sibling_clone)
			node->next_sibling_clone->prev_sibling_clone = node->prev_sibling_clone;
		      if (node->clone_of)
			node->former_clone_of = node->clone_of->decl;
		      node->clone_of = NULL;
		      node->next_sibling_clone = NULL;
		      node->prev_sibling_clone = NULL;
		    }
		  else
		    gcc_assert (!clone->in_other_partition);
		  node->analyzed = false;
		  changed = true;
		  cgraph_node_remove_callees (node);
		}
	    }
	  else
	    {
	      cgraph_remove_node (node);
	      changed = true;
	    }
	}
    }
  for (node = cgraph_nodes; node; node = node->next)
    {
      /* Inline clones might be kept around so their materializing allows further
         cloning.  If the function the clone is inlined into is removed, we need
         to turn it into normal cone.  */
      if (node->global.inlined_to
	  && !node->callers)
	{
	  gcc_assert (node->clones);
	  node->global.inlined_to = NULL;
	  update_inlined_to_pointer (node, node);
	}
      node->aux = NULL;
    }

  if (file)
    fprintf (file, "\n");

  /* We must release unused extern inlines or sanity checking will fail.  Rest of transformations
     are undesirable at -O0 since we do not want to remove anything.  */
  if (!optimize)
    return changed;

  if (file)
    fprintf (file, "Reclaiming variables:");
  for (vnode = varpool_nodes; vnode; vnode = vnext)
    {
      vnext = vnode->next;
      if (!vnode->needed)
        {
	  if (file)
	    fprintf (file, " %s", varpool_node_name (vnode));
	  varpool_remove_node (vnode);
	  changed = true;
	}
    }

  /* Now update address_taken flags and try to promote functions to be local.  */

  if (file)
    fprintf (file, "\nClearing address taken flags:");
  for (node = cgraph_nodes; node; node = node->next)
    if (node->address_taken
	&& !node->reachable_from_other_partition)
      {
	int i;
        struct ipa_ref *ref;
	bool found = false;
        for (i = 0; ipa_ref_list_refering_iterate (&node->ref_list, i, ref)
		    && !found; i++)
	  {
	    gcc_assert (ref->use == IPA_REF_ADDR);
	    found = true;
	  }
	if (!found)
	  {
	    if (file)
	      fprintf (file, " %s", cgraph_node_name (node));
	    node->address_taken = false;
	    changed = true;
	    if (cgraph_local_node_p (node))
	      {
		node->local.local = true;
		if (file)
		  fprintf (file, " (local)");
	      }
	  }
      }

  /* Reclaim alias pairs for functions that have disappeared from the
     call graph.  */
  remove_unreachable_alias_pairs ();

  return changed;
}

/* Discover variables that have no longer address taken or that are read only
   and update their flags.

   FIXME: This can not be done in between gimplify and omp_expand since
   readonly flag plays role on what is shared and what is not.  Currently we do
   this transformation as part of whole program visibility and re-do at
   ipa-reference pass (to take into account clonning), but it would
   make sense to do it before early optimizations.  */

void
ipa_discover_readonly_nonaddressable_vars (void)
{
  struct varpool_node *vnode;
  if (dump_file)
    fprintf (dump_file, "Clearing variable flags:");
  for (vnode = varpool_nodes; vnode; vnode = vnode->next)
    if (vnode->finalized && varpool_all_refs_explicit_p (vnode)
	&& (TREE_ADDRESSABLE (vnode->decl) || !TREE_READONLY (vnode->decl)))
      {
	bool written = false;
	bool address_taken = false;
	int i;
        struct ipa_ref *ref;
        for (i = 0; ipa_ref_list_refering_iterate (&vnode->ref_list, i, ref)
		    && (!written || !address_taken); i++)
	  switch (ref->use)
	    {
	    case IPA_REF_ADDR:
	      address_taken = true;
	      break;
	    case IPA_REF_LOAD:
	      break;
	    case IPA_REF_STORE:
	      written = true;
	      break;
	    }
	if (TREE_ADDRESSABLE (vnode->decl) && !address_taken)
	  {
	    if (dump_file)
	      fprintf (dump_file, " %s (addressable)", varpool_node_name (vnode));
	    TREE_ADDRESSABLE (vnode->decl) = 0;
	  }
	if (!TREE_READONLY (vnode->decl) && !address_taken && !written
	    /* Making variable in explicit section readonly can cause section
	       type conflict. 
	       See e.g. gcc.c-torture/compile/pr23237.c */
	    && DECL_SECTION_NAME (vnode->decl) == NULL)
	  {
	    if (dump_file)
	      fprintf (dump_file, " %s (read-only)", varpool_node_name (vnode));
	    TREE_READONLY (vnode->decl) = 1;
	  }
      }
  if (dump_file)
    fprintf (dump_file, "\n");
}

/* Return true when there is a reference to node and it is not vtable.  */
static bool
cgraph_address_taken_from_non_vtable_p (struct cgraph_node *node)
{
  int i;
  struct ipa_ref *ref;
  for (i = 0; ipa_ref_list_reference_iterate (&node->ref_list, i, ref); i++)
    {
      struct varpool_node *node;
      if (ref->refered_type == IPA_REF_CGRAPH)
	return true;
      node = ipa_ref_varpool_node (ref);
      if (!DECL_VIRTUAL_P (node->decl))
	return true;
    }
  return false;
}

/* COMDAT functions must be shared only if they have address taken,
   otherwise we can produce our own private implementation with
   -fwhole-program.  
   Return true when turning COMDAT functoin static can not lead to wrong
   code when the resulting object links with a library defining same COMDAT.

   Virtual functions do have their addresses taken from the vtables,
   but in C++ there is no way to compare their addresses for equality.  */

bool
cgraph_comdat_can_be_unshared_p (struct cgraph_node *node)
{
  if ((cgraph_address_taken_from_non_vtable_p (node)
       && !DECL_VIRTUAL_P (node->decl))
      || !node->analyzed)
    return false;
  if (node->same_comdat_group)
    {
      struct cgraph_node *next;

      /* If more than one function is in the same COMDAT group, it must
         be shared even if just one function in the comdat group has
         address taken.  */
      for (next = node->same_comdat_group;
	   next != node; next = next->same_comdat_group)
	if (cgraph_address_taken_from_non_vtable_p (node)
	    && !DECL_VIRTUAL_P (next->decl))
	  return false;
    }
  return true;
}

/* Return true when function NODE should be considered externally visible.  */

static bool
cgraph_externally_visible_p (struct cgraph_node *node)
{
  struct cgraph_node *alias;
  if (!node->local.finalized)
    return false;
  if (!DECL_COMDAT (node->decl)
      && (!TREE_PUBLIC (node->decl) || DECL_EXTERNAL (node->decl)))
    return false;

  return true;
}

/* Return true when variable VNODE should be considered externally visible.  */

static bool
varpool_externally_visible_p (struct varpool_node *vnode)
{
  struct varpool_node *alias;
  if (!DECL_COMDAT (vnode->decl) && !TREE_PUBLIC (vnode->decl))
    return false;

  return true;
}

/* Dissolve the same_comdat_group list in which NODE resides.  */

static void
dissolve_same_comdat_group_list (struct cgraph_node *node)
{
  struct cgraph_node *n = node, *next;
  do
    {
      next = n->same_comdat_group;
      n->same_comdat_group = NULL;
      n = next;
    }
  while (n != node);
}

/* Mark visibility of all functions.

   A local function is one whose calls can occur only in the current
   compilation unit and all its calls are explicit, so we can change
   its calling convention.  We simply mark all static functions whose
   address is not taken as local.

   We also change the TREE_PUBLIC flag of all declarations that are public
   in language point of view but we want to overwrite this default
   via visibilities for the backend point of view.  */

static unsigned int
function_and_variable_visibility (void)
{
  bool whole_program = false;
  struct cgraph_node *node;
  struct varpool_node *vnode;
  struct pointer_set_t *aliased_nodes = pointer_set_create ();
  struct pointer_set_t *aliased_vnodes = pointer_set_create ();
  unsigned i;
  alias_pair *p;    

  for (node = cgraph_nodes; node; node = node->next)
    {
      /* Frontends and alias code marks nodes as needed before parsing is finished.
	 We may end up marking as node external nodes where this flag is meaningless
	 strip it.  */
      if (node->needed
	  && (DECL_EXTERNAL (node->decl) || !node->analyzed))
	node->needed = 0;

      /* C++ FE on lack of COMDAT support create local COMDAT functions
	 (that ought to be shared but can not due to object format
	 limitations).  It is neccesary to keep the flag to make rest of C++ FE
	 happy.  Clear the flag here to avoid confusion in middle-end.  */
      if (DECL_COMDAT (node->decl) && !TREE_PUBLIC (node->decl))
        DECL_COMDAT (node->decl) = 0;
      /* For external decls stop tracking same_comdat_group, it doesn't matter
	 what comdat group they are in when they won't be emitted in this TU,
	 and simplifies later passes.  */
      if (node->same_comdat_group && DECL_EXTERNAL (node->decl))
	{
#ifdef ENABLE_CHECKING
	  struct cgraph_node *n;

	  for (n = node->same_comdat_group;
	       n != node;
	       n = n->same_comdat_group)
	      /* If at least one of same comdat group functions is external,
		 all of them have to be, otherwise it is a front-end bug.  */
	      gcc_assert (DECL_EXTERNAL (n->decl));
#endif
	  dissolve_same_comdat_group_list (node);
	}
      gcc_assert ((!DECL_WEAK (node->decl) && !DECL_COMDAT (node->decl))
      	          || TREE_PUBLIC (node->decl) || DECL_EXTERNAL (node->decl));
      if (cgraph_externally_visible_p (node))
        {
	  gcc_assert (!node->global.inlined_to);
	  node->local.externally_visible = true;
	}
      else
	node->local.externally_visible = false;
      if (!node->local.externally_visible && node->analyzed
	  && !DECL_EXTERNAL (node->decl))
	{
          struct cgraph_node *alias;
	  gcc_assert (whole_program || in_lto_p || !TREE_PUBLIC (node->decl));
	  cgraph_make_decl_local (node->decl);
	  node->resolution = LDPR_PREVAILING_DEF_IRONLY;
	  for (alias = node->same_body; alias; alias = alias->next)
	    cgraph_make_decl_local (alias->decl);
	  if (node->same_comdat_group)
	    /* cgraph_externally_visible_p has already checked all other nodes
	       in the group and they will all be made local.  We need to
	       dissolve the group at once so that the predicate does not
	       segfault though. */
	    dissolve_same_comdat_group_list (node);
	}
      node->local.local = cgraph_local_node_p (node);
    }
  for (vnode = varpool_nodes; vnode; vnode = vnode->next)
    {
      /* weak flag makes no sense on local variables.  */
      gcc_assert (!DECL_WEAK (vnode->decl)
      		  || TREE_PUBLIC (vnode->decl) || DECL_EXTERNAL (vnode->decl));
      /* In several cases declarations can not be common:

	 - when declaration has initializer
	 - when it is in weak
	 - when it has specific section
	 - when it resides in non-generic address space.
	 - if declaration is local, it will get into .local common section
	   so common flag is not needed.  Frontends still produce these in
	   certain cases, such as for:

	     static int a __attribute__ ((common))

	 Canonicalize things here and clear the redundant flag.  */
      if (DECL_COMMON (vnode->decl)
	  && (!(TREE_PUBLIC (vnode->decl) || DECL_EXTERNAL (vnode->decl))
	      || (DECL_INITIAL (vnode->decl)
		  && DECL_INITIAL (vnode->decl) != error_mark_node)
	      || DECL_WEAK (vnode->decl)
	      || DECL_SECTION_NAME (vnode->decl) != NULL
	      || ! (ADDR_SPACE_GENERIC_P
		    (TYPE_ADDR_SPACE (TREE_TYPE (vnode->decl))))))
	DECL_COMMON (vnode->decl) = 0;
    }
  for (vnode = varpool_nodes_queue; vnode; vnode = vnode->next_needed)
    {
      if (!vnode->finalized)
        continue;
      if (vnode->needed
	  && varpool_externally_visible_p (vnode))
	vnode->externally_visible = true;
      else
        vnode->externally_visible = false;
      if (!vnode->externally_visible)
	{
	  gcc_assert (in_lto_p || whole_program || !TREE_PUBLIC (vnode->decl));
	  cgraph_make_decl_local (vnode->decl);
	  vnode->resolution = LDPR_PREVAILING_DEF_IRONLY;
	}
     gcc_assert (TREE_STATIC (vnode->decl));
    }

  if (dump_file)
    {
      fprintf (dump_file, "\nMarking local functions:");
      for (node = cgraph_nodes; node; node = node->next)
	if (node->local.local)
	  fprintf (dump_file, " %s", cgraph_node_name (node));
      fprintf (dump_file, "\n\n");
      fprintf (dump_file, "\nMarking externally visible functions:");
      for (node = cgraph_nodes; node; node = node->next)
	if (node->local.externally_visible)
	  fprintf (dump_file, " %s", cgraph_node_name (node));
      fprintf (dump_file, "\n\n");
      fprintf (dump_file, "\nMarking externally visible variables:");
      for (vnode = varpool_nodes_queue; vnode; vnode = vnode->next_needed)
	if (vnode->externally_visible)
	  fprintf (dump_file, " %s", varpool_node_name (vnode));
      fprintf (dump_file, "\n\n");
    }
  cgraph_function_flags_ready = true;
  return 0;
}

/* Local function pass handling visibilities.  This happens before LTO streaming
   so in particular -fwhole-program should be ignored at this level.  */

static unsigned int
local_function_and_variable_visibility (void)
{
  return function_and_variable_visibility ();
}

struct simple_ipa_opt_pass pass_ipa_function_and_variable_visibility =
{
 {
  SIMPLE_IPA_PASS,
  "visibility",				/* name */
  NULL,					/* gate */
  local_function_and_variable_visibility,/* execute */
  NULL,					/* sub */
  NULL,					/* next */
  0,					/* static_pass_number */
  TV_CGRAPHOPT,				/* tv_id */
  0,	                                /* properties_required */
  0,					/* properties_provided */
  0,					/* properties_destroyed */
  0,					/* todo_flags_start */
  TODO_remove_functions | TODO_dump_cgraph
  | TODO_ggc_collect			/* todo_flags_finish */
 }
};

/* Hash a cgraph node set element.  */

static hashval_t
hash_cgraph_node_set_element (const void *p)
{
  const_cgraph_node_set_element element = (const_cgraph_node_set_element) p;
  return htab_hash_pointer (element->node);
}

/* Compare two cgraph node set elements.  */

static int
eq_cgraph_node_set_element (const void *p1, const void *p2)
{
  const_cgraph_node_set_element e1 = (const_cgraph_node_set_element) p1;
  const_cgraph_node_set_element e2 = (const_cgraph_node_set_element) p2;

  return e1->node == e2->node;
}

/* Create a new cgraph node set.  */

cgraph_node_set
cgraph_node_set_new (void)
{
  cgraph_node_set new_node_set;

  new_node_set = ggc_alloc_cgraph_node_set_def ();
  new_node_set->hashtab = htab_create_ggc (10,
					   hash_cgraph_node_set_element,
					   eq_cgraph_node_set_element,
					   NULL);
  new_node_set->nodes = NULL;
  return new_node_set;
}

/* Add cgraph_node NODE to cgraph_node_set SET.  */

void
cgraph_node_set_add (cgraph_node_set set, struct cgraph_node *node)
{
  void **slot;
  cgraph_node_set_element element;
  struct cgraph_node_set_element_def dummy;

  dummy.node = node;
  slot = htab_find_slot (set->hashtab, &dummy, INSERT);

  if (*slot != HTAB_EMPTY_ENTRY)
    {
      element = (cgraph_node_set_element) *slot;
      gcc_assert (node == element->node
		  && (VEC_index (cgraph_node_ptr, set->nodes, element->index)
		      == node));
      return;
    }

  /* Insert node into hash table.  */
  element = ggc_alloc_cgraph_node_set_element_def ();
  element->node = node;
  element->index = VEC_length (cgraph_node_ptr, set->nodes);
  *slot = element;

  /* Insert into node vector.  */
  VEC_safe_push (cgraph_node_ptr, gc, set->nodes, node);
}

/* Remove cgraph_node NODE from cgraph_node_set SET.  */

void
cgraph_node_set_remove (cgraph_node_set set, struct cgraph_node *node)
{
  void **slot, **last_slot;
  cgraph_node_set_element element, last_element;
  struct cgraph_node *last_node;
  struct cgraph_node_set_element_def dummy;

  dummy.node = node;
  slot = htab_find_slot (set->hashtab, &dummy, NO_INSERT);
  if (slot == NULL)
    return;

  element = (cgraph_node_set_element) *slot;
  gcc_assert (VEC_index (cgraph_node_ptr, set->nodes, element->index)
	      == node);

  /* Remove from vector. We do this by swapping node with the last element
     of the vector.  */
  last_node = VEC_pop (cgraph_node_ptr, set->nodes);
  if (last_node != node)
    {
      dummy.node = last_node;
      last_slot = htab_find_slot (set->hashtab, &dummy, NO_INSERT);
      last_element = (cgraph_node_set_element) *last_slot;
      gcc_assert (last_element);

      /* Move the last element to the original spot of NODE.  */
      last_element->index = element->index;
      VEC_replace (cgraph_node_ptr, set->nodes, last_element->index,
		   last_node);
    }

  /* Remove element from hash table.  */
  htab_clear_slot (set->hashtab, slot);
  ggc_free (element);
}

/* Find NODE in SET and return an iterator to it if found.  A null iterator
   is returned if NODE is not in SET.  */

cgraph_node_set_iterator
cgraph_node_set_find (cgraph_node_set set, struct cgraph_node *node)
{
  void **slot;
  struct cgraph_node_set_element_def dummy;
  cgraph_node_set_element element;
  cgraph_node_set_iterator csi;

  dummy.node = node;
  slot = htab_find_slot (set->hashtab, &dummy, NO_INSERT);
  if (slot == NULL)
    csi.index = (unsigned) ~0;
  else
    {
      element = (cgraph_node_set_element) *slot;
      gcc_assert (VEC_index (cgraph_node_ptr, set->nodes, element->index)
		  == node);
      csi.index = element->index;
    }
  csi.set = set;

  return csi;
}

/* Dump content of SET to file F.  */

void
dump_cgraph_node_set (FILE *f, cgraph_node_set set)
{
  cgraph_node_set_iterator iter;

  for (iter = csi_start (set); !csi_end_p (iter); csi_next (&iter))
    {
      struct cgraph_node *node = csi_node (iter);
      fprintf (f, " %s/%i", cgraph_node_name (node), node->uid);
    }
  fprintf (f, "\n");
}

/* Dump content of SET to stderr.  */

DEBUG_FUNCTION void
debug_cgraph_node_set (cgraph_node_set set)
{
  dump_cgraph_node_set (stderr, set);
}

/* Hash a varpool node set element.  */

static hashval_t
hash_varpool_node_set_element (const void *p)
{
  const_varpool_node_set_element element = (const_varpool_node_set_element) p;
  return htab_hash_pointer (element->node);
}

/* Compare two varpool node set elements.  */

static int
eq_varpool_node_set_element (const void *p1, const void *p2)
{
  const_varpool_node_set_element e1 = (const_varpool_node_set_element) p1;
  const_varpool_node_set_element e2 = (const_varpool_node_set_element) p2;

  return e1->node == e2->node;
}

/* Create a new varpool node set.  */

varpool_node_set
varpool_node_set_new (void)
{
  varpool_node_set new_node_set;

  new_node_set = ggc_alloc_varpool_node_set_def ();
  new_node_set->hashtab = htab_create_ggc (10,
					   hash_varpool_node_set_element,
					   eq_varpool_node_set_element,
					   NULL);
  new_node_set->nodes = NULL;
  return new_node_set;
}

/* Add varpool_node NODE to varpool_node_set SET.  */

void
varpool_node_set_add (varpool_node_set set, struct varpool_node *node)
{
  void **slot;
  varpool_node_set_element element;
  struct varpool_node_set_element_def dummy;

  dummy.node = node;
  slot = htab_find_slot (set->hashtab, &dummy, INSERT);

  if (*slot != HTAB_EMPTY_ENTRY)
    {
      element = (varpool_node_set_element) *slot;
      gcc_assert (node == element->node
		  && (VEC_index (varpool_node_ptr, set->nodes, element->index)
		      == node));
      return;
    }

  /* Insert node into hash table.  */
  element = ggc_alloc_varpool_node_set_element_def ();
  element->node = node;
  element->index = VEC_length (varpool_node_ptr, set->nodes);
  *slot = element;

  /* Insert into node vector.  */
  VEC_safe_push (varpool_node_ptr, gc, set->nodes, node);
}

/* Remove varpool_node NODE from varpool_node_set SET.  */

void
varpool_node_set_remove (varpool_node_set set, struct varpool_node *node)
{
  void **slot, **last_slot;
  varpool_node_set_element element, last_element;
  struct varpool_node *last_node;
  struct varpool_node_set_element_def dummy;

  dummy.node = node;
  slot = htab_find_slot (set->hashtab, &dummy, NO_INSERT);
  if (slot == NULL)
    return;

  element = (varpool_node_set_element) *slot;
  gcc_assert (VEC_index (varpool_node_ptr, set->nodes, element->index)
	      == node);

  /* Remove from vector. We do this by swapping node with the last element
     of the vector.  */
  last_node = VEC_pop (varpool_node_ptr, set->nodes);
  if (last_node != node)
    {
      dummy.node = last_node;
      last_slot = htab_find_slot (set->hashtab, &dummy, NO_INSERT);
      last_element = (varpool_node_set_element) *last_slot;
      gcc_assert (last_element);

      /* Move the last element to the original spot of NODE.  */
      last_element->index = element->index;
      VEC_replace (varpool_node_ptr, set->nodes, last_element->index,
		   last_node);
    }

  /* Remove element from hash table.  */
  htab_clear_slot (set->hashtab, slot);
  ggc_free (element);
}

/* Find NODE in SET and return an iterator to it if found.  A null iterator
   is returned if NODE is not in SET.  */

varpool_node_set_iterator
varpool_node_set_find (varpool_node_set set, struct varpool_node *node)
{
  void **slot;
  struct varpool_node_set_element_def dummy;
  varpool_node_set_element element;
  varpool_node_set_iterator vsi;

  dummy.node = node;
  slot = htab_find_slot (set->hashtab, &dummy, NO_INSERT);
  if (slot == NULL)
    vsi.index = (unsigned) ~0;
  else
    {
      element = (varpool_node_set_element) *slot;
      gcc_assert (VEC_index (varpool_node_ptr, set->nodes, element->index)
		  == node);
      vsi.index = element->index;
    }
  vsi.set = set;

  return vsi;
}

/* Dump content of SET to file F.  */

void
dump_varpool_node_set (FILE *f, varpool_node_set set)
{
  varpool_node_set_iterator iter;

  for (iter = vsi_start (set); !vsi_end_p (iter); vsi_next (&iter))
    {
      struct varpool_node *node = vsi_node (iter);
      fprintf (f, " %s", varpool_node_name (node));
    }
  fprintf (f, "\n");
}

/* Dump content of SET to stderr.  */

DEBUG_FUNCTION void
debug_varpool_node_set (varpool_node_set set)
{
  dump_varpool_node_set (stderr, set);
}

EXTERN_C_END
