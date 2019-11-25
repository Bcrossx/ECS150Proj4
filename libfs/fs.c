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
	char filename[FS_FILENAME_LEN];
	uint32_t filesize;
	uint16_t firstblock_index;
	uint8_t padding[10];
};

struct __attribute__((__packed__)) fsMeta
{
	struct superBlock superblock;
	uint16_t *fat;
	struct fileInfo rootDir[FS_FILE_MAX_COUNT];
};

static struct fsMeta* currFS;

bool mountedDisk;

int fs_mount(const char *diskname)
{
	//Open virtual disk
	int diskOpen = NULL;
	diskOpen = block_disk_open(diskname);

	//read in superblock on index 0
	block_read(0, &currFS->superblock);

	//malloc fat
	currFS->fat = malloc(superblock.fatblockCount * BLOCK_SIZE);

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
		mountedDisk = true;

	return 0;
}

int fs_umount(void)
{
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
		mountedDisk = false;
	if(block_disk_close() == -1)
		return -1;

	return 0;
}

int fs_info(void)
{
	//   FS Info:
	// total_blk_count=4100
	// fat_blk_count=2
	// rdir_blk=3
	// data_blk=4
	// data_blk_count=4096
	// fat_free_ratio=4095/4096
	// rdir_free_ratio=128/128
	if(mountedDisk = false) // Could cause an issue
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

int isFilenameInvalid(const char *filename){
	if(filename == NULL)
		return -1;
	if(sizeof(filename) > FS_FILENAME_LEN)
		return -1;

}

int fs_create(const char *filename)
{
	/* TODO: Phase 2 */
	// Create a new file
	// Initially, size is 0 and pointer to first data block is FAT_EOC

	if(isFilenameInvalid(filename))
		return -1;

	for(int i = currFS->superblock.rootDir_blockIndex; i < currFS->superblock.datablock_startIndex; i++){
		
		if(strcmp(filename, ))
	}
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
