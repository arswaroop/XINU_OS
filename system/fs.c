#include <xinu.h>
#include <kernel.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>


#ifdef FS
#include <fs.h>

static struct fsystem fsd;
int dev0_numblocks;
int dev0_blocksize;
char *dev0_blocks;

extern int dev0;

char block_cache[512];

#define SB_BLK 0
#define BM_BLK 1
#define RT_BLK 2

#define NUM_FD 16
struct filetable oft[NUM_FD];
int next_open_fd = 0;


#define INODES_PER_BLOCK (fsd.blocksz / sizeof(struct inode))
#define NUM_INODE_BLOCKS (( (fsd.ninodes % INODES_PER_BLOCK) == 0) ? fsd.ninodes / INODES_PER_BLOCK : (fsd.ninodes / INODES_PER_BLOCK) + 1)
#define FIRST_INODE_BLOCK 2

int fs_fileblock_to_diskblock(int dev, int fd, int fileblock);

/* YOUR CODE GOES HERE */

int fs_fileblock_to_diskblock(int dev, int fd, int fileblock) {
  int diskblock;

  if (fileblock >= INODEBLOCKS - 2) {
    printf("No indirect block support\n");
    return SYSERR;
  }

  diskblock = oft[fd].in.blocks[fileblock]; //get the logical block address

  return diskblock;
}

/* read in an inode and fill in the pointer */
int fs_get_inode_by_num(int dev, int inode_number, struct inode *in) {
  int bl, inn;
  int inode_off;

  if (dev != 0) {
    printf("Unsupported device\n");
    return SYSERR;
  }
  if (inode_number > fsd.ninodes) {
    printf("fs_get_inode_by_num: inode %d out of range\n", inode_number);
    return SYSERR;
  }

  bl = inode_number / INODES_PER_BLOCK;
  inn = inode_number % INODES_PER_BLOCK;
  bl += FIRST_INODE_BLOCK;

  inode_off = inn * sizeof(struct inode);

  /*
  printf("in_no: %d = %d/%d\n", inode_number, bl, inn);
  printf("inn*sizeof(struct inode): %d\n", inode_off);
  */

  bs_bread(dev0, bl, 0, &block_cache[0], fsd.blocksz);
  memcpy(in, &block_cache[inode_off], sizeof(struct inode));

  return OK;

}

int fs_put_inode_by_num(int dev, int inode_number, struct inode *in) {
  int bl, inn;

  if (dev != 0) {
    printf("Unsupported device\n");
    return SYSERR;
  }
  if (inode_number > fsd.ninodes) {
    printf("fs_put_inode_by_num: inode %d out of range\n", inode_number);
    return SYSERR;
  }

  bl = inode_number / INODES_PER_BLOCK;
  inn = inode_number % INODES_PER_BLOCK;
  bl += FIRST_INODE_BLOCK;

  /*
  printf("in_no: %d = %d/%d\n", inode_number, bl, inn);
  */

  bs_bread(dev0, bl, 0, block_cache, fsd.blocksz);
  memcpy(&block_cache[(inn*sizeof(struct inode))], in, sizeof(struct inode));
  bs_bwrite(dev0, bl, 0, block_cache, fsd.blocksz);

  return OK;
}
     
int fs_mkfs(int dev, int num_inodes) {
  int i;
  
  if (dev == 0) {
    fsd.nblocks = dev0_numblocks;
    fsd.blocksz = dev0_blocksize;
  }
  else {
    printf("Unsupported device\n");
    return SYSERR;
  }

  if (num_inodes < 1) {
    fsd.ninodes = DEFAULT_NUM_INODES;
  }
  else {
    fsd.ninodes = num_inodes;
  }

  i = fsd.nblocks;
  while ( (i % 8) != 0) {i++;}
  fsd.freemaskbytes = i / 8; 
  
  if ((fsd.freemask = getmem(fsd.freemaskbytes)) == (void *)SYSERR) {
    printf("fs_mkfs memget failed.\n");
    return SYSERR;
  }
  
  /* zero the free mask */
  for(i=0;i<fsd.freemaskbytes;i++) {
    fsd.freemask[i] = '\0';
  }
  
  fsd.inodes_used = 0;
  
  /* write the fsystem block to SB_BLK, mark block used */
  fs_setmaskbit(SB_BLK);
  bs_bwrite(dev0, SB_BLK, 0, &fsd, sizeof(struct fsystem));
  
  /* write the free block bitmask in BM_BLK, mark block used */
  fs_setmaskbit(BM_BLK);
  bs_bwrite(dev0, BM_BLK, 0, fsd.freemask, fsd.freemaskbytes);

  return 1;
}

void fs_print_fsd(void) {

  printf("fsd.ninodes: %d\n", fsd.ninodes);
  printf("sizeof(struct inode): %d\n", sizeof(struct inode));
  printf("INODES_PER_BLOCK: %d\n", INODES_PER_BLOCK);
  printf("NUM_INODE_BLOCKS: %d\n", NUM_INODE_BLOCKS);
}

/* specify the block number to be set in the mask */
int fs_setmaskbit(int b) {
  int mbyte, mbit;
  mbyte = b / 8;
  mbit = b % 8;

  fsd.freemask[mbyte] |= (0x80 >> mbit);
  return OK;
}

/* specify the block number to be read in the mask */
int fs_getmaskbit(int b) {
  int mbyte, mbit;
  mbyte = b / 8;
  mbit = b % 8;

  return( ( (fsd.freemask[mbyte] << mbit) & 0x80 ) >> 7);
  return OK;

}

/* specify the block number to be unset in the mask */
int fs_clearmaskbit(int b) {
  int mbyte, mbit, invb;
  mbyte = b / 8;
  mbit = b % 8;

  invb = ~(0x80 >> mbit);
  invb &= 0xFF;

  fsd.freemask[mbyte] &= invb;
  return OK;
}

/* This is maybe a little overcomplicated since the lowest-numbered
   block is indicated in the high-order bit.  Shift the byte by j
   positions to make the match in bit7 (the 8th bit) and then shift
   that value 7 times to the low-order bit to print.  Yes, it could be
   the other way...  */
void fs_printfreemask(void) {
  int i,j;

  for (i=0; i < fsd.freemaskbytes; i++) {
    for (j=0; j < 8; j++) {
      printf("%d", ((fsd.freemask[i] << j) & 0x80) >> 7);
    }
    if ( (i % 8) == 7) {
      printf("\n");
    }
  }
  printf("\n");
}

int min(int a, int b)
{
	if(a > b) 
		return b;
	return a;
}
int fs_open(char *filename, int flags) {
	
	// Check length of filename
	if(strlen(filename) > FILENAMELEN)
	{
		printf("The length of the filename exceeds FILENAMELEN\n");
		return SYSERR;
	}

	int i, j, fd=-1;
	for (i=0; i<fsd.root_dir.numentries; i++)
	{
		if(strcmp(filename, fsd.root_dir.entry[i]) == 0)
		{
			break;
		}
	}
	if(i== fsd.root_dir.numentries)
	{
		printf("No such file exists\n");
		return SYSERR;
	}
	// Get the index of file in open file table
	for(j=0; j<NUM_FD; j++)
	{
		if(strcmp(fsd.root_dir.entry[i].name, oft[i].de->name) == 0)
		{
			fd = j;
		}
	}

	if (fd == -1)
	{
		printf("Entry not found in file table\n");
		return SYSERR;
	}
	// Check if the file is already open
	if(oft[fd].state == FSTATE_OPEN)
	{
		printf("The file is already open");
		return SYSERR;
	}
	
	struct inode in;
	int status = fs_get_inode_by_num(0, oft[fd].in.id, &in);
	if(status == SYSERR)
	{
		printf("Error in fs_get_inode_by_num\n");
		return SYSERR;		
	}
	
	oft[fd].state = FSTATE_OPEN;
	oft[fd].fileptr = 0;
	oft[fd].de = &fsd.root_dir.entry[i];
	oft[fd].in = in;

	return fd;
}

int fs_close(int fd) {

	if(fd < 0 || fd > NUM_FD)
	{
		printf("File not valid\n");
		return SYSERR;
	}

	if(oft[fd].state == FSTATE_CLOSED)
	{
		printf("\nFile already closed");
		return SYSERR;
	}
	oft[fd].state = FSTATE_CLOSED;
	oft[fd].fileptr = 0;
	return OK;
}

int fs_create(char *filename, int mode) {
	
	int i;
	// Check mode
	if (mode!= O_CREAT)
	{
		printf("Please enter valid mode\n");
		return SYSERR;
	}
	
	if(strlen(filename) > FILENAMELEN )
	{
		printf("The file name is lengthy\n");
		return SYSERR;
	}
	
	// Check the root directory for the filename
	for(i=0; i<fsd.root_dir.numentries; i++)
	{
		if(strcmp(fsd.root_dir.entry[i].name, filename) == 0)
		{
			printf("The file with this name already exists\n");
			return SYSERR;
		}
	}
	// Look for empty inodes
	if(fsd.inodes_used >= fsd.ninodes)
	{
		printf("There are no inoeds available at  this moment \n");
		return SYSERR;	
	}

	struct inode in;
	int status = fs_get_inode_by_num(0, ++fsd.inodes_used, &in);
	
	if(status == SYSERR)
	{
		printf("Error in fetching an inode at ::fs_get_inode_by_num\n");
		return SYSERR;	
	}
	
	in.id = fsd.inodes_used;
	in.type = INODE_TYPE_FILE;
	in.nlink = 0;
	in.device = 0;
	in.size = 0;
	
	// Write the inode back to memory
	status = fs_put_inode_by_num(0, fsd.inodes_used, &in);
	if (status == SYSERR)
	{
		printf("Error in fs_put_inode_by_num \n");
		return SYSERR;
	}

	
	strcpy(fsd.root_dir.entry[fsd.root_dir.numentries].name, filename);
	fsd.root_dir.entry[fsd.root_dir.numentries].inode_num =	fsd.inodes_used;

	oft[fsd.inodes_used].state = FSTATE_OPEN;
	oft[fsd.inodes_used].fileptr = 0;
	oft[fsd.inodes_used].de = &fsd.root_dir.entry[fsd.root_dir.numentries];  	
	fsd.root_dir.numentries++;
	oft[fsd.inodes_used].in = in;
	return fsd.inodes_used;
}

int fs_seek(int fd, int offset) {
	
	if(fd < 0 || fd > NUM_FD)
	{
		printf("Please input a valid file\n");
		return SYSERR;
	}
	
	if(oft[fd].state != FSTATE_OPEN)
	{
		printf("Please open the file for seek operation\n");
		return SYSERR;
	}

	oft[fd].fileptr += offset;
	return oft[fd].fileptr;
}

int fs_read(int fd, void *buf, int nbytes) {

	if (fd < 0 || fd > NUM_FD)
	{
		printf("Please input a valid file\n");
		return SYSERR;
	}

	// Sanity check to see if the function is open
	if(oft[fd].state != FSTATE_OPEN )
	{
		printf("Open file in read or write mode\n");
		return SYSERR;
	}
	
	if(nbytes<0)
	{
		printf("Please provide valid number of bytes to read");
		return SYSERR;
	}

	// Check if file is empty
	if(oft[fd].in.size == 0)
	{
		printf("The file to read is empty\n");
		return SYSERR;	
	}

	nbytes += oft[fd].fileptr;
	int blocksToRead = nbytes / MDEV_BLOCK_SIZE;

	// Add a block if a few bytes are remaining
	if(nbytes % MDEV_BLOCK_SIZE !=0)
	{
		blocksToRead++;
	}

	blocksToRead = min(oft[fd].in.size, blocksToRead);
	// Set the first block to read
	int i = (oft[fd].fileptr/MDEV_BLOCK_SIZE);	
	memset(buf, NULL, (MDEV_BLOCK_SIZE * MDEV_NUM_BLOCKS));
	
	// Setting the offset
	int offset = oft[fd].fileptr % MDEV_BLOCK_SIZE;
	int bytesRead = 0, temp = 0;
	// Reading from i until blocksToRead
	for (offset; i< blocksToRead; i++)
	{
		// Clear Cache
		memset(block_cache, NULL, MDEV_BLOCK_SIZE+1);
		
		if(bs_bread(0, oft[fd].in.blocks[i], offset, block_cache, MDEV_BLOCK_SIZE - offset) == SYSERR)
		{
			printf("Error in reading file\n");
			return SYSERR;
		}
		// Copy the contents of cache to buffer
		strcpy((buf+temp), block_cache);
		temp = strlen(buf);
		bytesRead += temp;
		offset = 0; // Reset the offset to 0
	}
	// Reset the file pointer to new value
	oft[fd].fileptr = bytesRead;
 	return bytesRead;
	
}

int fs_write(int fd, void *buf, int nbytes) {
	
	int i=0;
	if(fd < 0 || fd >NUM_FD)
	{
		printf("Please provide a valid file\n");
		return SYSERR;
	}

	if(oft[fd].state != FSTATE_OPEN )
	{
		printf("Open file in write mode\n");
		return SYSERR;
	}

	if(nbytes < 0)
	{
		printf("Please input valid number of bytes\n");
		return SYSERR;
	}
	
	struct inode temp;
	// Overwrite the previous content
	// Hence clear the content in the inodes that has been previously set
	if((oft[fd].in.size) > 0)
	{
		temp = oft[fd].in;
		while(oft[fd].in.size > 0)
		{
			if(fs_clearmaskbit(temp.blocks[oft[fd].in.size -1 ]) != OK)
			{
				printf("Cannot clear the block. Error in fs_clearmaskbit\n");
				return SYSERR;
			}
			oft[fd].in.size--;
		}
	}
	
	int blocks_to_write = nbytes/MDEV_BLOCK_SIZE;
	// Add a block if required
	if(nbytes % MDEV_BLOCK_SIZE != 0)
	{
		blocks_to_write++;
	}
	
	int bytes_to_write = nbytes;
	
	int current_block;
	//Writing ahead of the current file pointer
	if (oft[fd].fileptr > 0)
	{
		current_block = (oft[fd].fileptr/MDEV_BLOCK_SIZE);
	}
	int j = oft[fd].in.blocks[current_block];
	for(i = 0; i<blocks_to_write && j<MDEV_BLOCK_SIZE;i++ )
	{
		j = oft[fd].in.blocks[current_block];
		// Replace the contents of the block
		if(j == NULL)
		{
			break;
		}
		// Clearing the block
		memset(block_cache, NULL, MDEV_BLOCK_SIZE);
		if(bs_bwrite(0, j, 0, block_cache, MDEV_BLOCK_SIZE) == SYSERR)
                {
			printf("Error in writing to the block : bs_bwrite\n");
			return SYSERR;
		}	
		int minBytes = min(MDEV_BLOCK_SIZE, bytes_to_write);
		// COPYING THE CONTENT FROM BUFFER TO CACHE
		memcpy(block_cache, buf, minBytes);
		//WRITING TO THE BLOCKS FROM CACHE			
		if(bs_bwrite(0, j, 0, block_cache, MDEV_BLOCK_SIZE) == SYSERR)
		{
			printf("Error in writing to the block: bs_bwrite \n");
			return SYSERR;
		}
		buf = (char*) buf + minBytes;
		bytes_to_write -= minBytes;
		current_block++;
	}
	
	// Get the first block to write the data to
	j = FIRST_INODE_BLOCK + NUM_INODE_BLOCKS;
	for(i=current_block; i<blocks_to_write && j<MDEV_BLOCK_SIZE; j++)
	{
		// CHECK IF BLOCK IS FREE
		if(fs_getmaskbit(j) == 0)
		{
			// CLEAR THE CACHE
			memset(block_cache, NULL, MDEV_BLOCK_SIZE);
			
			// CLEARING THE BLOCKS		
			if(bs_bwrite(0, j, 0, block_cache, MDEV_BLOCK_SIZE) == SYSERR)
			{
				printf("Error in writing to the block : bs_bwrite\n");
				return SYSERR;
			}
			
			int minBytes = min(MDEV_BLOCK_SIZE, bytes_to_write);
			
			// COPYING THE CONTENT FROM BUFFER TO CACHE
			memcpy(block_cache, buf, minBytes);
			
			//WRITING TO THE BLOCKS FROM CACHE
			if(bs_bwrite(0, j, 0, block_cache, MDEV_BLOCK_SIZE) == SYSERR)
			{
				printf("Error in writing to the block: bs_bwrite \n");
				return SYSERR;
			}
			
			buf = (char*) buf + minBytes;
			bytes_to_write -= minBytes;
			// SET THE MASK FOR THE BLOCK
			fs_setmaskbit(j);
			oft[fd].in.blocks[i++] = j;
		}
	}

	oft[fd].in.size = blocks_to_write;
	if(fs_put_inode_by_num(0, oft[fd].in.id, &oft[fd].in) == SYSERR)
	{
		printf("Error in: fs_put_inode_by_num\n");
		return SYSERR;
	}
	oft[fd].fileptr = nbytes;
	return nbytes;
	
}

#endif /* FS */
