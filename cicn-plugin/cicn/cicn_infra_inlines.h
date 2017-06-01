/*
 * Copyright (c) 2016 by Cisco Systems Inc. All Rights Reserved.
 *
 */
#ifndef _CICN_INFRA_INLINES_H_
#define _CICN_INFRA_INLINES_H_ 1

#if !CICN_VPP_PLUGIN
#error "cicn-internal file included externally"
#endif

#include <vlib/vlib.h>

/*
 * Wrapper for buffer allocation that returns pointer rather than index
 */
static inline vlib_buffer_t *
cicn_infra_vlib_buffer_alloc (vlib_main_t * vm)
{
  vlib_buffer_t *b0;
  u32 bi0;
  if (vlib_buffer_alloc (vm, &bi0, 1) != 1)
    {
      b0 = 0;
      goto done;
    }
  b0 = vlib_get_buffer (vm, bi0);

done:
  return (b0);
}

/*
 * Wrapper for buffer free that uses pointer rather than index
 */
static inline void
cicn_infra_vlib_buffer_free (vlib_buffer_t * b0, vlib_main_t * vm)
{
  u32 bi0 = vlib_get_buffer_index (vm, b0);
  vlib_buffer_free_one (vm, bi0);
}

#endif // CICN_INFRA_INLINES_H_
