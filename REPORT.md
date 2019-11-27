# ECS 150 Project 4 Report
## Implementation
### Category: Structures

A big part of this program was making sure we had a strong set of data
structures that were not too difficult to access. We learned that we needed to
make our structs "packed" so that we could better control the data allocation.
For the first couple phases we only needed the FS meta information, later in
phase 3 the file descriptor sctruct was added to hold our file descriptor
information.  

### Category: Mounting/unmounting

Initially this phase seemed fairly simple, but there were a few tricky parts
which tripped us up. Deciding how much data was needed to be malloc'd was
particularly interesting. As opposed to previous projects it took a lot more
thought and planning to figure out the amount of data that we needed to allocate
for the individual sets of FS Meta information. It was mainly difficult because
I wasn't sure of how to think of it at first, but the FS table from the
discussion slides really helped us to grasp these concepts and make it easier
to decide how much to allocate as well as figuring out where to read/write to.
Once we had that we just used a global flag so the other functions could tell if
the FS was mounted or not.

### Category: File creation/deletion

File Creation and deletion were fairly easy. The only issue we faced was in
addressing the memory of the root directory entry. Filename was validated initially
by checking for length and whether it existed already in root directory. Deletion is 
done by modifying both rootblock and FAT block. The above sanity checks were done for
deletion as well. fs_ls() was fairly straightforward to implement

### Category: File descriptor operations

Of these functions fs_open() was the only one I struggled to figure out. After
trying some different things I finally figured out this was actually pretty straightforward. 
Search the file descriptor table for empty file descriptor
entries, then search the root directory for the specified filename, set the
offset to 0 and point the file_info to that root directory entry. We also used a
counter for fs_open() and fs_close() to make sure there weren't more than our
max number of file descriptors.

Our file descriptor also includes a data_block_idx element to keep track of the datablock
that the offset belongs to. This will become useful when write operations come into play
later

### Category: File reading/writing

Reading/Writing was the hardest part of the assignment. The main complication arose when
the offsets are not aligned with the datablocks. When reading and writing, the 
following scenarios are possible:

- the offset starts at the beginning of the block
- the offset starts at the middle of the block
- the offset starts at the very end of the file

#### fs_read()

The getNumBlocks() function gives us the no. of full and partial data blocks to read in.
For the first and last blocks, we read the blocks into a temporary buffer, since a partial
read may occur, which is then transferred to the buffer given to us. The middle blocks are
read completely and so we read in the entire block size. 

#### fs_write()

Writing involves either overwriting a given datablock or appending to an already full file.
In the latter case, empty datablocks must be found and then allocated to the file by modifying
the fat file. The filesize of a given file must also be periodically modified, but only if
we are appending to the file. Similar to read, the blocks are read in for the blocks that
already exist. If we are allocating a new datablock, the data can be written directly from the
buffer. Overall, writing was more complicated than reading and had a few bugs to take care of.

##    Outside Sources
For function references:
* [Linux man pages](https://linux.die.net/man/)
