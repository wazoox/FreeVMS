/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 1999, 2001, 2003  Free Software Foundation, Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifdef FSYS_ODS2

#include "shared.h"
#include "filesys.h"

#include "mytypes.h"
#include "fiddef.h"
#include "uicdef.h"
#include "fatdef.h"
#include "dirdef.h"
#include "fh2def.h"
#include "fm2def.h"
#include "hm2def.h"

#define BLOCKSIZE 512
#define MAXREC (BLOCKSIZE - 2)

#if 0
#define STRUCT_DIR_SIZE (sizeof(struct _dir)) // but this gives one too much
#else
#define STRUCT_DIR_SIZE 7 
#endif

/* sizes are always in bytes, BLOCK values are always in DEV_BSIZE (sectors) */
#define DEV_BSIZE 512

/* include/linux/fs.h */
#define BLOCK_SIZE 512		/* initial block size for superblock read */
/* made up, defaults to 1 but can be passed via mount_opts */
#define HOME_BLOCK 1

#define home_block ((struct _hm2 *)(FSYS_BUF))
#define index_header ((struct _fh2 *)(FSYS_BUF + 0x200))
#define mfd_header ((struct _fh2 *)(FSYS_BUF + 0x400))
#define mfd ((struct _fh2 *)(FSYS_BUF + 0x600))
#define file_header ((struct _fh2 *)(FSYS_BUF + 0x800))
#define dir ((struct _dir *)(FSYS_BUF + 0xa00))

int mymemcmp(char * s, char * t, int size) {
  for (;size;size--,s++,t++)
    if ((*s)!=(*t)) return 1;
  return 0;
}

int
ods2_mount (void)
{
  int retval = 1;

  devread (HOME_BLOCK, 0, 512, (char *) home_block);

  if ((((current_drive & 0x80) || (current_slice != 0))
       && (current_slice != PC_SLICE_TYPE_ODS2))
      || !devread (HOME_BLOCK, 0, 512, (char *) home_block)
      || mymemcmp(home_block->hm2$t_format,"DECFILE11B  ",12) != 0)
      retval = 0;

  devread(VMSLONG(home_block->hm2$l_ibmaplbn) + VMSWORD(home_block->hm2$w_ibmapsize), 0, 512, (char *) index_header);

  devread(VMSLONG(home_block->hm2$l_ibmaplbn) + VMSWORD(home_block->hm2$w_ibmapsize) + (4 - 1), 0, 512, (char *) mfd_header);

  devread(VMSLONG(home_block->hm2$l_ibmaplbn) + VMSWORD(home_block->hm2$w_ibmapsize) + (4 - 1), 0, 512, (char *) file_header);

  filepos = 0;
  ods2_read((char *) mfd, 512);

  return retval;
}

static int
get_fm2_val(unsigned short ** mpp, unsigned long * phyblk, unsigned long *phylen) {
  unsigned short *mp=*mpp;
  if (phyblk==0 || phylen==0) return -1;
	switch (VMSWORD(*mp) >> 14) {
	case FM2$C_PLACEMENT:
	  *phylen = 0;
	  (*mpp)++;
	  break;
	case FM2$C_FORMAT1:
	  *phylen = (VMSWORD(*mp) & 0377) + 1;
	  *phyblk = ((VMSWORD(*mp) & 037400) << 8) | VMSWORD(mp[1]);
	  (*mpp) += 2;
	  break;
	case FM2$C_FORMAT2:
	  *phylen = (VMSWORD(*mp) & 037777) + 1;
	  *phyblk = (VMSWORD(mp[2]) << 16) | VMSWORD(mp[1]);
	  (*mpp) += 3;
	  break;
	case FM2$C_FORMAT3:
	  *phylen = ((VMSWORD(*mp) & 037777) << 16) + VMSWORD(mp[1]) + 1;
	  *phyblk = (VMSWORD(mp[3]) << 16) | VMSWORD(mp[2]);
	  (*mpp) += 4;
	default:
	  return 0;
	}
	return 1;
}

static int
ods2_block_map (int logical_block)
{
  unsigned curvbn=0; // should be 1, but I guess grub starts at 0
  unsigned short *me;
  unsigned short *mp = (unsigned short *) file_header + file_header->fh2$b_mpoffset;
  me = mp + file_header->fh2$b_map_inuse;

  while (mp < me) {
    unsigned long phyblk, phylen;
    get_fm2_val(&mp,&phyblk,&phylen);
    //    printf("get %x %x %x %x %x %x",mp,phyblk,phylen,curvbn,logical_block,file_header->fh2$b_map_inuse);
    if (logical_block>=curvbn && logical_block<(curvbn+phylen))
      return phyblk+logical_block-curvbn;
    if (phylen!=0) {
      curvbn += phylen;
    }
  }

  return(0);
}

int
ods2_read (char *buf, int len)
{
  int logical_block;
  int offset;
  int map;
  int ret = 0;
  int size = 0;

  while (len > 0)
    {
      /* find the (logical) block component of our location */
      logical_block = filepos >> 9;
      offset = filepos & (512 - 1);
      map = ods2_block_map (logical_block);
      if (map < 0)
	break;

      size = 512;
      size -= offset;
      if (size > len)
	size = len;

      disk_read_func = disk_read_hook;

      devread (map, offset, size, buf);

      disk_read_func = NULL;

      buf += size;
      len -= size;
      filepos += size;
      ret += size;
    }

  if (errnum)
    ret = 0;

  return ret;
}

static int
ods2_index_block_map (int logical_block)
{
  unsigned curvbn=1; // should be 1, but I guess grub starts at 0
  unsigned short *me;
  unsigned short *mp = (unsigned short *) index_header + index_header->fh2$b_mpoffset;
  me = mp + index_header->fh2$b_map_inuse;

  while (mp < me) {
    unsigned long phyblk, phylen;
    get_fm2_val(&mp,&phyblk,&phylen);
    //printf("get %x %x %x %x %x %x",mp,phyblk,phylen,curvbn,logical_block,index_header->fh2$b_map_inuse);
    if (logical_block>=curvbn && logical_block<(curvbn+phylen))
      return phyblk+logical_block-curvbn;
    if (phylen!=0) {
      curvbn += phylen;
    }
  }

  return(0);
}

int
ods2_dir (char *dirname)
{
  //printf("dir X%sX\n",dirname);
  void *tmp;
  tmp = mfd;
  struct _dir * dr = tmp;
  
  while (1) {
    char * rest, ch;
    int str_chk;

    if (!*dirname || isspace (*dirname))
      {
	struct _fiddef * fid = &file_header->fh2$w_fid;
	//printf("fid %x %x %x\n",fid->fid$w_num,fid->fid$b_nmx,fid->fid$w_seq);
	struct _fatdef * fat = &file_header->fh2$w_recattr;
	filemax = (VMSSWAP(fat->fat$l_efblk)<<9)-512+fat->fat$w_ffbyte;
	//printf("filemax %x\n",filemax);
	return 1;
      }

    while (*dirname == '/')
      dirname++;

    if ((VMSLONG(file_header->fh2$l_filechar) & FH2$M_DIRECTORY)==0)
      {
	errnum = ERR_BAD_FILETYPE;
	return 0;
      }

    /* skip to next slash or end of filename (space) */
    for (rest = dirname; (ch = *rest) && !isspace (ch) && ch != '/';
	 rest++);

    *rest = 0;

    do {

      if (VMSWORD(dr->dir$w_size) > MAXREC)
	{
	  //printf("dr %x %x %x %x\n",dr,dr->dir$w_size,dr->dir$w_verlimit,dr->dir$b_namecount); 
	  if (print_possibilities < 0)
	    {
	    }
	  else
	    {
	      errnum = ERR_FILE_NOT_FOUND;
	      *rest = ch;
	    }
	  return (print_possibilities < 0);
	}

      int saved_c = dr->dir$t_name[dr->dir$b_namecount];
      dr->dir$t_name[dr->dir$b_namecount]=0;
      str_chk = substring(dirname,dr->dir$t_name);

# ifndef STAGE1_5
      if (print_possibilities && ch != '/'
	  && (!*dirname || str_chk <= 0))
	{
	  if (print_possibilities > 0)
	    print_possibilities = -print_possibilities;
	  print_a_completion (dr->dir$t_name);
	}
# endif

      dr->dir$t_name[dr->dir$b_namecount]=saved_c;

      dr = (void *) ((char *) dr + VMSWORD(dr->dir$w_size) + 2);

    } while (/*!dp->inode ||*/ (str_chk || (print_possibilities && ch != '/')));
	tmp = (char*) dr-sizeof(struct _dir1);
    struct _dir1 *de = tmp; //(struct _dir1 *) (dr->dir$t_name + ((dr->dir$b_namecount + 1) & ~1));
    struct _fiddef * fid = &de->dir$fid;
    int filenum = fid->fid$w_num + (fid->fid$b_nmx<<16);
    int phy = ods2_index_block_map(VMSLONG(home_block->hm2$w_ibmapvbn) + VMSWORD(home_block->hm2$w_ibmapsize) + filenum - 1);
    //printf("filen %x %x %x %x %x\n",filenum,fid->fid$w_num,fid->fid$b_nmx,fid->fid$w_seq,phy);
    devread(phy, 0, 512, (char *) file_header);
    //devread(VMSLONG(home_block->hm2$l_ibmaplbn) + VMSWORD(home_block->hm2$w_ibmapsize) + (filenum - 1), 0, 512, file_header);
    filepos = 0;
    ods2_read((char *) dir, 512);
    filepos = 0;
    dr = dir;
    *(dirname = rest) = ch;
  }
}

#endif /* FSYS_ODS2 */
