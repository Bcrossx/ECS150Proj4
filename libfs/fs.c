#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "disk.h"
#include "fs.h"

#define FAT_EOC 0xFFFF

struct fileDesc //File descriptors struct
{
  size_t offset;
  struct fileInfo* file_info;
  int root_idx;
  int open; //count < max
};

struct __attribute__((__packed__)) superBlock //superblock struct
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

struct __attribute__((__packed__)) fsMeta //meta info
{
  struct superBlock superblock;
  struct fileInfo rootDir [FS_FILE_MAX_COUNT];
  uint16_t *fat;
  struct fileDesc fd_table [FS_OPEN_MAX_COUNT];
};

static struct fsMeta* currFS;

bool mountedDisk = false;

int get_FAT_free(){ //gets number of free FAT entries
	if(!mountedDisk)
		return -1;

	int count = 0;
	for(int i = 1; i < currFS->superblock.datablockCount; i++){
		if(currFS->fat[i] == 0)
			count++;
	}
	return count;
}

int get_root_free(){ //gets number of free root directory entries
	if(!mountedDisk)
		return -1;

	int count = 0;
	for(int i = 0; i < FS_FILE_MAX_COUNT; i++){
		if(strcmp("\0", (char *)currFS->rootDir[i].filename) == 0){
			count++;
    }
  }
  return count;
}

int isFilenameInvalid(const char *filename){ //checks for valid filename

	if(filename == NULL || filename[0] == '\0')
		return -1;
	for(int i = 1; i < FS_FILENAME_LEN; i++){
		if(filename[i] == '\0')
			return 0;
	}
	return -1;
}

int fs_mount(const char *diskname)
{
  // Open the virtual disk
  // Read the metadata (superblock, fat, root directory)

  //Open virtual disk
  int diskOpen;
  diskOpen = block_disk_open(diskname);

  currFS = malloc(sizeof(struct fsMeta));

  //read in superblock on index 0
  block_read(0, &currFS->superblock);

  char *signature = "ECS150FS";
  //check if block begins with signature
  if(memcmp(&currFS->superblock.signature, signature, 8) != 0)
    return -1;

  // check if disk size matches
  if(currFS->superblock.totalblockCount != block_disk_count())
    return -1;

  int fat = 0;

  //malloc fat
  currFS->fat = malloc(currFS->superblock.fatblockCount * BLOCK_SIZE);

  if(currFS->fat == NULL)
		return -1;

  for(fat = 1; fat <= currFS->superblock.fatblockCount; fat++){ //iterate through fat entries

      //First argument is the index, fat starts at index 1
      //Second argument is the amount we want to read
      block_read(fat, currFS->fat+(BLOCK_SIZE/2)*(fat-1));

  }

  //read in root directory
  block_read(currFS->superblock.rootDir_blockIndex, (void *)&currFS->rootDir);

  if(diskOpen == -1) //virtual disk cannot be opened
    return -1;

  if(diskOpen != 0){
    mountedDisk = false;
  }else if(diskOpen == 0){
    mountedDisk = true; //if disk open set boolean variable to true
  }

  return 0;
}

int fs_umount(void)
{
  if(mountedDisk == false) // disk not mounted
    return -1;

  //for this function we need to update any changes made with the Write
  //function. We start with the fat because the superblock cannot be changed.
  for(int fat = 1; fat <= currFS->superblock.fatblockCount; fat++){ //iterate through fat entries
      //Same as mount but write instead of read
      block_write(fat, currFS->fat+(BLOCK_SIZE/2)*(fat-1));

  }

  //write root directory
  block_write(currFS->superblock.rootDir_blockIndex, currFS->rootDir);

  //close disk
  if(mountedDisk == true){
      block_disk_close();
      mountedDisk = false;
  }

  return 0;

}

int fs_info(void)
{
  //Show information about volume

  if(mountedDisk == false){ //check if disk mounted
    return -1;
  }

  printf("FS Info: \n");
  printf("total_blk_count=%d\n", currFS->superblock.totalblockCount);
	printf("fat_blk_count=%d\n", currFS->superblock.fatblockCount);
	printf("rdir_blk=%d\n", currFS->superblock.rootDir_blockIndex);
	printf("data_blk=%d\n", currFS->superblock.datablock_startIndex);
	printf("data_blk_count=%d\n", currFS->superblock.datablockCount);
	printf("fat_free_ratio=%d/%d\n", get_FAT_free(), currFS->superblock.datablockCount);
	printf("rdir_free_ratio=%d/%d\n",  get_root_free(), FS_FILE_MAX_COUNT);

  return 0;
}

int fs_create(const char *filename)
{
  // Create a new file
  // Initially, size is 0 and pointer to first data block is FAT_EOC
  if(!mountedDisk || isFilenameInvalid(filename))
  		return -1;

  	int emptyEntry = -1;
  	for(int i = 0; i < FS_FILE_MAX_COUNT; i++){
  		if(strcmp(filename, (char *) currFS->rootDir[i].filename) == 0)
  			return -1;
  		if(strcmp("\0", (char *) currFS->rootDir[i].filename) == 0)
  			emptyEntry = i;
  	}

  	if(emptyEntry == -1)
  		return -1;

  	struct fileInfo empty = currFS->rootDir[emptyEntry];
  	strcpy((char *) empty.filename, filename);
  	empty.firstblock_index = FAT_EOC;
  	empty.filesize = 0;

  	return 0;
}

int fs_delete(const char *filename)
{
  // Delete an existing file
  // Free allocated data blocks, if any
  if(isFilenameInvalid(filename))
  return -1;

  //check if file is opened later

  for(int i = 0; i < FS_FILE_MAX_COUNT; i++){
    if(strcmp(filename, (char *) currFS->rootDir[i].filename) == 0) {
      int currBlock = currFS->rootDir[i].firstblock_index;
      while(currBlock != FAT_EOC){
        int nextBlock = currFS->fat[currBlock];
        currFS->fat[currBlock] = 0;
        currBlock = nextBlock;
      }
      currFS->fat[currBlock] = 0;
      strcpy((char *) currFS->rootDir[i].filename, "\0");
      return 0;
    }
  }

return -1;
}

int fs_ls(void)
{
  //List all the existing files
  if(!mountedDisk) // virtual disk not opened
		return -1;

	printf("FS Ls:\n");
	for(int i = 0; i < FS_FILE_MAX_COUNT; i++){
		if(strcmp((char *) currFS->rootDir[i].filename, "\0") != 0)
			printf("file: %s, size: %d, data_blk: %d\n", (char *) currFS->rootDir[i].filename, currFS->rootDir[i].filesize, currFS->rootDir[i].firstblock_index);
	}
	return 0;
}

int fs_open(const char *filename)
{
  // Initialize and return file descriptor
  // Can open same file multiple times
  // Contains file's offset (initially 0)

  int fd = 0; //file descriptor

  for(int i = 0; i < FS_OPEN_MAX_COUNT; i++){ // Search for empty file descriptors
    if(currFS->fd_table[i].file_info == NULL){
      fd = i; //found empty fd_table spot
      break;
    }
  }

  for(int j = 0; j < FS_FILE_MAX_COUNT; j++){ //search root dir for *filename
    if(currFS->rootDir[j].filename == filename){
      currFS->fd_table[fd].offset = 0; //set offset to 0
      currFS->fd_table[fd].file_info = currFS->rootDir+j; //point file_info to found root dir entry
        break;
    }
    if(j == FS_FILE_MAX_COUNT){
      return -1;
    }
  }

  int openf = currFS->fd_table[fd].open; //counter for open fd's, max 32
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
  if(currFS->fd_table[fd].file_info == NULL){ //check if already NULL
    return 0;
  }else{
    currFS->fd_table[fd].file_info = NULL; //if not already NULL, set to NULL
  }

  int openf = currFS->fd_table[fd].open; //decrement counter after closing
  --openf;
  if(openf < 0){
    return -1;
  }
  //error checking

  return 0;
}

int fs_stat(int fd)
{
  if(!mountedDisk)
		return -1;

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
  // Write a certain number of bytes to a file
// Extend file if necessary

  // lseek(fd, block_nr * BLOCK_SIZE);
   // write(fd, buf, BLOCK_SIZE);

   return 0;
}

int fs_read(int fd, void *buf, size_t count)
{
  // Read a certain number of bytes from a file
  // lseek(fd, block_nr * BLOCK_SIZE);
  // read(fd, buf, BLOCK_SIZE);
  return 0;
}
