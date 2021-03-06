/* Code dealing with register stack frames, for GDB, the GNU debugger.

   Copyright (C) 1986, 1987, 1988, 1989, 1990, 1991, 1992, 1993, 1994, 1995,
   1996, 1997, 1998, 1999, 2000, 2001, 2002, 2007, 2008, 2009
   Free Software Foundation, Inc.

   This file is part of GDB.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */


#include "defs.h"
#include "regcache.h"
#include "sentinel-frame.h"
#include "inferior.h"
#include "frame-unwind.h"

struct frame_unwind_cache
{
  struct regcache *regcache;
};

void *
sentinel_frame_cache (struct regcache *regcache)
{
  struct frame_unwind_cache *cache = 
    FRAME_OBSTACK_ZALLOC (struct frame_unwind_cache);
  cache->regcache = regcache;
  return cache;
}

/* Here the register value is taken direct from the register cache.  */

static struct value *
sentinel_frame_prev_register (struct frame_info *this_frame,
			      void **this_prologue_cache,
			      int regnum)
{
  struct gdbarch *gdbarch = get_frame_arch (this_frame);
  struct frame_unwind_cache *cache = *this_prologue_cache;
  struct value *value;

  /* Return the actual value.  */
  value = allocate_value (register_type (gdbarch, regnum));
  VALUE_LVAL (value) = lval_register;
  VALUE_REGNUM (value) = regnum;
  VALUE_FRAME_ID (value) = get_frame_id (this_frame);

  /* Use the regcache_cooked_read() method so that it, on the fly,
     constructs either a raw or pseudo register from the raw
     register cache.  */
  regcache_cooked_read (cache->regcache, regnum, value_contents_raw (value));

  return value;
}

static void
sentinel_frame_this_id (struct frame_info *this_frame,
			void **this_prologue_cache,
			struct frame_id *this_id)
{
  /* The sentinel frame is used as a starting point for creating the
     previous (inner most) frame.  That frame's THIS_ID method will be
     called to determine the inner most frame's ID.  Not this one.  */
  internal_error (__FILE__, __LINE__, _("sentinel_frame_this_id called"));
}

static struct gdbarch *
sentinel_frame_prev_arch (struct frame_info *this_frame,
			  void **this_prologue_cache)
{
  struct frame_unwind_cache *cache = *this_prologue_cache;
  return get_regcache_arch (cache->regcache);
}

const struct frame_unwind sentinel_frame_unwinder =
{
  SENTINEL_FRAME,
  sentinel_frame_this_id,
  sentinel_frame_prev_register,
  NULL,
  NULL,
  NULL,
  sentinel_frame_prev_arch,
};

const struct frame_unwind *const sentinel_frame_unwind = &sentinel_frame_unwinder;
