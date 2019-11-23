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

//struct fileDescriptors

//file system structs (superblock,FAT)
struct __attribute__((__packed__)) superBlock
{
  unint8_t signature [8];
  unint16_t totalblockCount;
  unint16_t rootDir_blockIndex;
  unint16_t datablock_startIndex;
  unint16_t datablockCount;
  unint8_t fatblockCount;
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

bool mountedDisk;

int fs_mount(const char *diskname)
{
  // Open the virtual disk
  // Read the metadata (superblock, fat, root directory)

  //Open virtual disk
  int diskOpen = NULL;
  diskOpen = block_disk_open(diskname);

  //read in superblock on index 0
  block_read(0, currFS.superblock);

  //malloc fat
  currFS.fat = malloc(superblock.fatblockCount * BLOCK_SIZE);

  for(int fat = 1; fat <= superblock.fatblockCount; fat++){ //iterate through fat entries

      //First argument is the index, fat starts at index 1
      //Second argument is the amount we want to read
      block_read(fat, currFS.fat+(BLOCK_SIZE/2)*(fat-1));

  }

  //read in root directory
  block_read(rootDir_blockIndex, currFS.rootDir);


  if(diskOpen == -1)
    return -1;
  if(diskOpen != NULL) //if disk open set boolean variable to true
    mountedDisk = TRUE;

  return 0;
}

int fs_umount(void)
{
  //Close virtual disk (make sure that virtual disk is up-to-date)

  //for this function we need to update any changes made with the Write
  //function. We start with the fat because the superblock cannot be changed.
  for(int fat = 1; fat <= superblock.fatblockCount; fat++){ //iterate through fat entries

      block_write(fat, currFS.fat+(BLOCK_SIZE/2)*(fat-1));

  }

  //write root directory
  block_write(rootDir_blockIndex, currFS.rootDir);

  //close disk
  block_disk_close();

  if(block_disk_close() != -1)
    mountedDisk = FALSE;
  if(block_disk_close() == -1)
    return -1;

  return 0;

}

int fs_info(void)
{
  //Show information about volume

//   FS Info:
// total_blk_count=4100
// fat_blk_count=2
// rdir_blk=3
// data_blk=4
// data_blk_count=4096
// fat_free_ratio=4095/4096
// rdir_free_ratio=128/128
  if(mountedDisk = FALSE) // Could cause an issue
    return -1;
  printf("\t FS Info: \n");
  printf("total_blk_count='%d'", totalblockCount);
  printf("fat_blk_count='%d'", fatblockCount);
  printf("rdir_blk='%d'", rootDir);
  printf("data_blk='%d'", datablock_startIndex);
  printf("data_blk_count='%d'", datablockCount);
  printf("fat_free_ratio='%d'", BLOCK_SIZE-fatblockCount "/" BLOCK_SIZE);
  printf("rdir_free_ratio='%d'",  "/" );
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
