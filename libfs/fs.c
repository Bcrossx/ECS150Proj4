#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "disk.h"
#include "fs.h"

/* TODO: Phase 1 */
 // For phase1 start by defining the data structures corresponding to the
 // blocks containing the meta-information
 // about the file system (superblock, FAT and root directory)

//file system structs (superblock,FAT)
struct __attribute__((__packed__)) superBlock
{
  unint8_t signature [8];
  unint2_t totalblockCount;
  unint2_t rootDir_blockIndex;
  unint2_t datablock_startIndex;
  unint2_t datablockCount;
  unint2_t fatblockCount;
  unint8_t padding[4079];
};

struct __attribute__((__packed__)) fileInfo //rootentry/rootdirec as an array
{
  char filename[FS_FILENAME_LEN];
  uint32_t filesize;
  uint16_t firstblock_index;
  unint8_t padding[10];
};

struct __attribute__((__packed__)) fsMeta
{
  struct superBlock superblock;
  struct fileInfo rootDir [FS_FILE_MAX_COUNT];
  uint16_t *fat;
};

static struct fsMeta* currFS;

int fs_mount(const char *diskname)
{
	/* TODO: Phase 1 */
  // Open the virtual disk
  // Read the metadata (superblock, fat, root directory)
}

int fs_umount(void)
{
	/* TODO: Phase 1 */
  //Close virtual disk (make sure that virtual disk is up-to-date)

}

int fs_info(void)
{
	/* TODO: Phase 1 */
  //Show information about volume

}

int fs_create(const char *filename)
{
	/* TODO: Phase 2 */
  // Create a new file
  // Initially, size is 0 and pointer to first data block is FAT_EOC

}

int fs_delete(const char *filename)
{
	/* TODO: Phase 2 */
  // Delete an existing file
// Free allocated data blocks, if any
}

int fs_ls(void)
{
	/* TODO: Phase 2 */
  //List all the existing files

}

int fs_open(const char *filename)
{
  // Initialize and return file descriptor
// 32 file descriptors max
// Can open same file multiple times
// Contains file's offset (initially 0)

//fd = open(*filename, flags);

}

int fs_close(int fd)
{
	/* TODO: Phase 3 */
  // Close file descriptor
}

int fs_stat(int fd)
{
	/* TODO: Phase 3 */
  // Return file's size

}

int fs_lseek(int fd, size_t offset)
{
	/* TODO: Phase 3 */
  // Move file's offset
}

// Write/read most difficult
// Don't leave for last minute
int fs_write(int fd, void *buf, size_t count)
{
	/* TODO: Phase 4 */
  // Write a certain number of bytes to a file
// Extend file if necessary

  // lseek(fd, block_nr * BLOCK_SIZE);
   // write(fd, buf, BLOCK_SIZE);
}

int fs_read(int fd, void *buf, size_t count)
{
	/* TODO: Phase 4 */
  // Read a certain number of bytes from a file
  // lseek(fd, block_nr * BLOCK_SIZE);
  // read(fd, buf, BLOCK_SIZE);
}
