#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "disk.h"
#include "fs.h"

/* TODO: Phase 1 */
 // For phase1 start by defining the data structures corresponding to the
 // blocks containing the meta-information
 // about the file system (superblock, FAT and root directory)

struct fileDesc
{
  size_t offset;
  struct fileInfo* file_info;
  int open; //count < max
};

//file system structs (superblock,FAT)
struct __attribute__((__packed__)) superBlock
{
  uint8_t signature [8];
  uint16_t totalblockCount;
  uint16_t rootDir_blockIndex;
  uint16_t datablock_startIndex;
  uint16_t datablockCount;
  uint8_t fatblockCount;
  uint8_t padding[4079];
};

struct __attribute__((__packed__)) fileInfo //rootentry/rootdirec as an array
{
  char filename[FS_FILENAME_LEN];
  uint32_t filesize;
  uint16_t firstblock_index;
  uint8_t padding[10];
};

struct __attribute__((__packed__)) fsMeta
{
  struct superBlock superblock;
  struct fileInfo rootDir [FS_FILE_MAX_COUNT];
  uint16_t *fat;
  struct fileDesc fd_table [FS_OPEN_MAX_COUNT];
};

static struct fsMeta* currFS;

bool mountedDisk = false;

int fs_mount(const char *diskname)
{
  // Open the virtual disk
  // Read the metadata (superblock, fat, root directory)

  //Open virtual disk
  int diskOpen;
  diskOpen = block_disk_open(diskname);

  //read in superblock on index 0
  block_read(0, &currFS->superblock);

  int fat = 0;

  //malloc fat
  currFS->fat = malloc(currFS->superblock.fatblockCount * BLOCK_SIZE);

  for(fat = 1; fat <= currFS->superblock.fatblockCount; fat++){ //iterate through fat entries

      //First argument is the index, fat starts at index 1
      //Second argument is the amount we want to read
      block_read(fat, currFS->fat+(BLOCK_SIZE/2)*(fat-1));

  }

  //read in root directory
  block_read(currFS->superblock.rootDir_blockIndex, currFS->rootDir);


  if(diskOpen == -1)
    return -1;
  if(!diskOpen){
    mountedDisk = false;
  }else
    mountedDisk = true;//if disk open set boolean variable to true

  return 0;
}

int fs_umount(void)
{
  //Close virtual disk (make sure that virtual disk is up-to-date)

  //for this function we need to update any changes made with the Write
  //function. We start with the fat because the superblock cannot be changed.
  for(int fat = 1; fat <= currFS->superblock.fatblockCount; fat++){ //iterate through fat entries

      block_write(fat, currFS->fat+(BLOCK_SIZE/2)*(fat-1));

  }

  //write root directory
  block_write(currFS->superblock.rootDir_blockIndex, currFS->rootDir);

  //close disk
  block_disk_close();

  if(block_disk_close() != -1)
    mountedDisk = false;
  if(block_disk_close() == -1)
    return -1;

  return 0;

}

int fs_info(void)
{
  //Show information about volume

  //struct *currFS;
//   FS Info:
// total_blk_count=4100
// fat_blk_count=2
// rdir_blk=3
// data_blk=4
// data_blk_count=4096
// fat_free_ratio=4095/4096
// rdir_free_ratio=128/128
  if(mountedDisk == false){ // Could cause an issue
    return -1;
  }
  printf("\t FS Info: \n");
  printf("total_blk_count='%d'", currFS->superblock.totalblockCount);
  printf("fat_blk_count='%d'", currFS->superblock.fatblockCount);
  printf("rdir_blk='%d'", currFS->superblock.rootDir_blockIndex);
  printf("data_blk='%d'", currFS->superblock.datablock_startIndex);
  printf("data_blk_count='%d'", currFS->superblock.datablockCount);
  printf("fat_free_ratio='%d'/'%d'", BLOCK_SIZE-currFS->superblock.fatblockCount, BLOCK_SIZE);
  printf("rdir_free_ratio='%d'/'%d'", currFS->superblock.rootDir_blockIndex-FS_OPEN_MAX_COUNT, FS_FILE_MAX_COUNT);
  return 0;
}

int fs_create(const char *filename)
{
	/* TODO: Phase 2 */
  // Create a new file
  // Initially, size is 0 and pointer to first data block is FAT_EOC

  return 0;
}

int fs_delete(const char *filename)
{
	/* TODO: Phase 2 */
  // Delete an existing file
// Free allocated data blocks, if any
  return 0;
}

int fs_ls(void)
{
	/* TODO: Phase 2 */
  //List all the existing files

  return 0;
}

int fs_open(const char *filename)
{
  // Initialize and return file descriptor
  // 32 file descriptors max
  // Can open same file multiple times
  // Contains file's offset (initially 0)

  //first empty file descripts (find first null)
  //search root dir for *filename
  //point file info to the found root dir entry
  //set offset to 0

  int fd = 0; //file descriptor

  for(int i = 0; i < FS_OPEN_MAX_COUNT; i++){
    if(currFS->fd_table[i].file_info == NULL){
      fd = i; //found empty fd_table spot
      break;
    }
  }

  for(int j = 0; j < FS_FILE_MAX_COUNT; j++){
    if(currFS->rootDir[j].filename == filename){
      currFS->fd_table[fd].offset = 0; //set offset to 0
      currFS->fd_table[fd].file_info = currFS->rootDir+j; //point file_info to found root dir entry
        break;
    }
    if(j == FS_FILE_MAX_COUNT){
      return -1;
    }
  }

  int openf = currFS->fd_table[fd].open;
  if(openf < FS_OPEN_MAX_COUNT){
    ++openf;
  }else{
    return -1;
  }

  return fd;
}

int fs_close(int fd)
{
  // Close file descriptor

  if(mountedDisk == false){ //check if fs is mounted (mountedDisk = true)
    return -1;
  }
    if(currFS->fd_table[fd].file_info == NULL){
      return 0;
    }else{
      currFS->fd_table[fd].file_info = NULL;
    }

  int openf = currFS->fd_table[fd].open;
  --openf;
  if(openf < 0){
    return -1;
  }
  //error checking

  return 0;
}

int fs_stat(int fd)
{
  //error checking
  return currFS->fd_table[1].file_info->filesize;
}

int fs_lseek(int fd, size_t offset)
{
  // Move file's offset
  // Just modify offset

  //check if offset is less than size of file
  currFS->fd_table[fd].offset = offset;
  //error checking

  return 0;
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

   return 0;
}

int fs_read(int fd, void *buf, size_t count)
{
	/* TODO: Phase 4 */
  // Read a certain number of bytes from a file
  // lseek(fd, block_nr * BLOCK_SIZE);
  // read(fd, buf, BLOCK_SIZE);
  return 0;
}
