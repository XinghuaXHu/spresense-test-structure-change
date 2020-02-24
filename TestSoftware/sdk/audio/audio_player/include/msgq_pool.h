/*
 * msgq_pool.h -- Message queue pool layout definition.
 *
 * This file was created by msgq_layout.conf
 * !!! CAUTION! don't edit this file manually !!!
 *
 *   Notes: (C) Copyright 2018 Sony Semiconductor Solutions Corporation
 */

#ifndef MSGQ_POOL_H_INCLUDED
#define MSGQ_POOL_H_INCLUDED

#include "msgq_id.h"

extern const MsgQueDef MsgqPoolDefs[NUM_MSGQ_POOLS] = {
   /* n_drm, n_size, n_num, h_drm, h_size, h_num */
  { 0x00000000, 0, 0, 0x00000000, 0, 0, 0 }, /* MSGQ_NULL */
  { 0xfe374, 60, 10, 0xffffffff, 0, 0 }, /* MSGQ_AUD_MNG */
  { 0xfe5cc, 40, 2, 0xffffffff, 0, 0 }, /* MSGQ_AUD_APP */
  { 0xfe61c, 20, 5, 0xffffffff, 0, 0 }, /* MSGQ_AUD_DSP */
  { 0xfe680, 20, 5, 0xffffffff, 0, 0 }, /* MSGQ_AUD_PFDSP0 */
  { 0xfe6e4, 20, 5, 0xffffffff, 0, 0 }, /* MSGQ_AUD_PFDSP1 */
  { 0xfe748, 48, 5, 0xffffffff, 0, 0 }, /* MSGQ_AUD_PLY */
  { 0xfe838, 48, 5, 0xffffffff, 0, 0 }, /* MSGQ_AUD_SUB_PLY */
  { 0xfe928, 48, 8, 0xffffffff, 0, 0 }, /* MSGQ_AUD_OUTPUT_MIX */
  { 0xfeaa8, 32, 16, 0xffffffff, 0, 0 }, /* MSGQ_AUD_RND_PLY */
  { 0xfeca8, 16, 8, 0xffffffff, 0, 0 }, /* MSGQ_AUD_RND_PLY_SYNC */
  { 0xfed28, 32, 16, 0xffffffff, 0, 0 }, /* MSGQ_AUD_RND_SUB */
  { 0xfef28, 16, 8, 0xffffffff, 0, 0 }, /* MSGQ_AUD_RND_SUB_SYNC */
};

#endif /* MSGQ_POOL_H_INCLUDED */
