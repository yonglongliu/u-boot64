/*
 * Copyright (c) 2015 Grzegorz Kostka (kostka.grzegorz@gmail.com)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - The name of the author may not be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <ext4_config.h>
#include <ext4_blockdev.h>
#include <ext4_errno.h>
#include <mmc.h>
#include <asm/arch/sdio_cfg.h>

static block_dev_desc_t *ext4fs_block_dev_desc = NULL;
static disk_partition_t *part_info = NULL;
void lwext4_set_blk_dev(block_dev_desc_t *rbdd, disk_partition_t *info)
{
    assert(rbdd->blksz == (1 << rbdd->log2blksz));
    ext4fs_block_dev_desc = rbdd;
    part_info = info;
}

/**********************BLOCKDEV INTERFACE**************************************/
static int blockdev_open(struct ext4_blockdev *bdev);
static int blockdev_bread(struct ext4_blockdev *bdev, void *buf, uint64_t blk_id,
             uint32_t blk_cnt);
static int blockdev_bwrite(struct ext4_blockdev *bdev, const void *buf,
              uint64_t blk_id, uint32_t blk_cnt);
static int blockdev_close(struct ext4_blockdev *bdev);
//static int blockdev_lock(struct ext4_blockdev *bdev);
//static int blockdev_unlock(struct ext4_blockdev *bdev);

/******************************************************************************/
EXT4_BLOCKDEV_STATIC_INSTANCE(blockdev, 512, 0, blockdev_open,
                  blockdev_bread, blockdev_bwrite, blockdev_close,
                  0, 0);

/******************************************************************************/
static int blockdev_open(struct ext4_blockdev *bdev)
{
    blockdev.part_offset = part_info->start * part_info->blksz;
    //I stuck here for a long time
    blockdev.part_size = (uint64_t)part_info->blksz * part_info->size;
    blockdev.bdif->ph_bcnt = part_info->size;
    return EOK;
}

/******************************************************************************/

static int blockdev_bread(struct ext4_blockdev *bdev, void *buf, uint64_t blk_id,
             uint32_t blk_cnt)
{
    /*blockdev_bread: skeleton*/
    unsigned long ret = 0;
    struct mmc *mmc = find_mmc_device(EMMC);
    int part_num = 0;
    if (!mmc)
        return EIO;
    /*userdata part num is 0 according to function of mmc_set_capacity*/
    if(mmc->part_num != 0) {
        part_num = mmc_switch_part(EMMC, 0);
        mmc->part_num = part_num;
    }
    //printf("Emmc_Read partnum=%d,startBlock=%llx,blk_cnt=%u,buf=%p\n",part_num, blk_id, blk_cnt, buf);
    ret = ext4fs_block_dev_desc->block_read(EMMC,blk_id,blk_cnt,buf);
    if ((size_t)ret != blk_cnt)
        return EIO;
    return EOK;
}


/******************************************************************************/
static int blockdev_bwrite(struct ext4_blockdev *bdev, const void *buf,
              uint64_t blk_id, uint32_t blk_cnt)
{
    /*blockdev_bwrite: skeleton*/
    unsigned long ret = 0;
    struct mmc *mmc = find_mmc_device(EMMC);
    int part_num = 0;
    if (!mmc)
        return EIO;
    /*userdata part num is 0 according to function of mmc_set_capacity*/
    if(mmc->part_num != 0) {
        part_num = mmc_switch_part(EMMC, 0);
        mmc->part_num = part_num;
    }
   // printf("Emmc_Write partnum=%d,startBlock=%llx,blk_cnt=%u,buf=%p\n",part_num,blk_id,blk_cnt,buf);
    ret = ext4fs_block_dev_desc->block_write(EMMC,blk_id,blk_cnt,buf);
    if ((size_t)ret != blk_cnt)
        return EIO;
    return EOK;
}
/******************************************************************************/
static int blockdev_close(struct ext4_blockdev *bdev)
{
    /*blockdev_close: skeleton*/
    (void)bdev;
    return EOK;
}

// static int blockdev_lock(struct ext4_blockdev *bdev)
// {
//     /*blockdev_lock: skeleton*/
//     return EIO;
// }

// static int blockdev_unlock(struct ext4_blockdev *bdev)
// {
//     /*blockdev_unlock: skeleton*/
//     return EIO;
// }

/******************************************************************************/
struct ext4_blockdev *ext4_blockdev_get(void)
{
    return &blockdev;
}
/******************************************************************************/

