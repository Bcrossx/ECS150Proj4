#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "disk.h"
#include "fs.h"

#define FAT_EOC 0xFFFF

/* TODO: Phase 1 */
 // For phase1 start by defining the data structures corresponding to the
 // blocks containing the meta-information
 // about the file system (superblock, FAT and root directory)

//file system structs (superblock,FAT)
struct __attribute__((__packed__)) superBlock
{
	uint8_t signature[8];
	uint16_t totalblockCount;
	uint16_t rootDir_blockIndex;
	uint16_t datablock_startIndex;
	uint16_t datablockCount;
	uint8_t fatblockCount;
	uint8_t padding[4079];
};

struct __attribute__((__packed__)) fileInfo 
{
	uint8_t filename[FS_FILENAME_LEN];
	uint32_t filesize;
	uint16_t firstblock_index;
	uint8_t padding[10];
};

struct __attribute__((__packed__)) fsMeta
{
	struct superBlock superblock;
	uint16_t *fat;
	struct fileInfo *rootDir;
};

static struct fsMeta currFS;

static bool mountedDisk = false;

int get_FAT_free(){
	if(!mountedDisk)
		return -1;

	int count = 0;
	for(int i = 0; i < currFS.superblock.datablockCount; i++){
		if(currFS.fat[i] == 0)
			count++;
	}
	return count;
}

int get_root_free(){
	if(!mountedDisk)
		return -1;

	int count = 0;
	for(int i = 0; i < FS_FILE_MAX_COUNT; i++){
		if(strcmp("\0", (char *) currFS.rootDir[i].filename) == 0)
			count++;
	}
	return count;
}

int isFilenameInvalid(const char *filename){
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
	char *signature = "ECS150FS";
	//Open virtual disk
	if(block_disk_open(diskname) == -1)
		return -1;

	//read in superblock on index 0
	block_read(0, &currFS.superblock);

	//check if block begins with signature
	if(memcmp(&currFS.superblock.signature, signature, 8) != 0)
		return -1;
	
	// check if disk size matches
	if(currFS.superblock.totalblockCount != block_disk_count())
		return -1;

	//malloc fat
	currFS.fat = malloc(currFS.superblock.datablockCount * sizeof(uint16_t));
	if(currFS.fat == NULL)
		return -1;

	for(int fat = 1; fat < currFS.superblock.fatblockCount + 1; fat++){ //iterate through fat entries
		//First argument is the index, fat starts at index 1
		//Second argument is the amount we want to read
		if(block_read(fat, currFS.fat + (BLOCK_SIZE/2) * (fat - 1)) == -1){
			return -1;
		}
	}

	//read in root directory
	currFS.rootDir = malloc(FS_FILE_MAX_COUNT * sizeof(struct fileInfo));
	if(block_read(currFS.superblock.rootDir_blockIndex, currFS.rootDir) == -1)
		return -1;

	mountedDisk = true;
	return 0;
}

int fs_umount(void)
{
	if(!mountedDisk) // disk hasn't been mounted
		return -1;
		
	//for this function we need to update any changes made with the Write
	//function. We start with the fat because the superblock cannot be changed.
	for(int fat = 1; fat < currFS.superblock.fatblockCount + 1; fat++){ //iterate through fat entries
		if(block_write(fat, currFS.fat + (BLOCK_SIZE/2) * (fat-1)) == -1)
			return -1;
	}

	//write to root directory
	if(block_write(currFS.superblock.rootDir_blockIndex, currFS.rootDir) == -1)
		return -1;

	//close disk
	if(block_disk_close() == -1)
		return -1;

	mountedDisk = false;
	return 0;
}

int fs_info(void)
{
	if(!mountedDisk) // disk hasn't been mounted
		return -1;

	printf("FS Info:\n");
	printf("total_blk_count=%d\n", currFS.superblock.totalblockCount);
	printf("fat_blk_count=%d\n", currFS.superblock.fatblockCount);
	printf("rdir_blk=%d\n", currFS.superblock.rootDir_blockIndex);
	printf("data_blk=%d\n", currFS.superblock.datablock_startIndex);
	printf("data_blk_count=%d\n", currFS.superblock.datablockCount);
	printf("fat_free_ratio=%d/%d\n", get_FAT_free(), currFS.superblock.datablockCount);
	printf("rdir_free_ratio=%d/%d\n",  get_root_free(), FS_FILE_MAX_COUNT);
	return 0;
}

int fs_create(const char *filename)
{
	/* TODO: Phase 2 */
	// Create a new file
	// Initially, size is 0 and pointer to first data block is FAT_EOC

	if(!mountedDisk || isFilenameInvalid(filename))
		return -1;
	
	int emptyEntry = -1;
	for(int i = 0; i < FS_FILE_MAX_COUNT; i++){
		if(strcmp(filename, (char *) currFS.rootDir[i].filename) == 0)
			return -1;
		if(strcmp("\0", (char *) currFS.rootDir[i].filename) == 0)
			emptyEntry = i;
	}

	if(emptyEntry == -1)
		return -1;
		
	struct fileInfo empty = currFS.rootDir[emptyEntry];
	strcpy((char *) empty.filename, filename);
	empty.firstblock_index = FAT_EOC;
	empty.filesize = 0;
	
	return 0;
}

int fs_delete(const char *filename)
{
	/* TODO: Phase 2 */
	// Delete an existing file
	// Free allocated data blocks, if any
	
	if(isFilenameInvalid(filename))
		return -1;
	
	//check if file is opened later

	for(int i = 0; i < FS_FILE_MAX_COUNT; i++){
		if(strcmp(filename, (char *) currFS.rootDir[i].filename) == 0) {
			int currBlock = currFS.rootDir[i].firstblock_index;
			while(currBlock != 0){
				int nextBlock = currFS.fat[currBlock];
				currFS.fat[currBlock] = 0;
				currBlock = nextBlock;
			}
			strcpy("\0", (char *) currFS.rootDir[i].filename);
			return 0;
		}
	}
	
	return -1;
}

int fs_ls(void)
{
	/* TODO: Phase 2 */
	//List all the existing files
	if(!mountedDisk) // virtual disk not opened
		return -1;
	
	printf("FS Ls:\n");
	for(int i = 0; i < FS_FILE_MAX_COUNT; i++){
		if(strcmp((char *) currFS.rootDir[i].filename, "\0") != 0)
			printf("file: %s, size: %d, data_blk: %d\n", (char *) currFS.rootDir[i].filename, currFS.rootDir[i].filesize, currFS.rootDir[i].firstblock_index);
	}
	return 0;
}

int fs_open(const char *filename)
{
	// Initialize and return file descriptor
// 32 file descriptors max
// Can open same file multiple times
// Contains file's offset (initially 0)

//fd = open(*filename, flags);
	return 0;
}

int fs_close(int fd)
{
	/* TODO: Phase 3 */
	// Close file descriptor
	return 0;
}

int fs_stat(int fd)
{
	/* TODO: Phase 3 */
	// Return file's size
	return 0;

}

int fs_lseek(int fd, size_t offset)
{
	/* TODO: Phase 3 */
	// Move file's offset
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
