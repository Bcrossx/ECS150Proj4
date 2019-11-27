# File system
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


### Category: File descriptor operations

Of these functions fs_open() was the only one I struggled to figure out. After
trying some different things I finally figured out this was actually pretty straightforward. Search the file descriptor table for empty file descriptor
entries, then search the root directory for the specified filename, set the
offset to 0 and point the file_info to that root directory entry. We also used a
counter for fs_open() and fs_close() to make sure there weren't more than our
max number of file descriptors.

### Category: File reading/writing

##    Outside Sources
For function references:
* [Linux man pages](https://linux.die.net/man/)
