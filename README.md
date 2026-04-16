PES-VCS — Version Control System 
Objective

This project implements a simplified version control system similar to Git. It tracks file changes, stores snapshots efficiently using content-addressable storage, and supports commit history.

Features Implemented
pes init → Initialize repository
pes add <file> → Stage files
pes status → Show file states
pes commit -m "msg" → Create commit
pes log → Show commit history
Project Structure

.pes/
├── objects/
├── refs/
│ └── heads/
│ └── main
├── index
└── HEAD

Phase 1 — Object Storage

Implementation:

Stored file contents as blobs using SHA-256
Used content-addressable storage
Implemented deduplication
Stored objects in .pes/objects/xx/hash

Screenshots:

Output of ./test_objects
Output of find .pes/objects -type f
Phase 2 — Tree Objects

Implementation:

Built directory structure from index
Supported nested directories
Stored tree objects linking blobs

Screenshots:

Output of ./test_tree
Output of xxd on tree object
Phase 3 — Index (Staging Area)

Implementation:

Text-based index file
Stored mode, hash, mtime, size, path
Implemented index_add, index_load, index_save

Screenshots:

pes init, pes add, pes status
cat .pes/index
Phase 4 — Commits

Implementation:

Built tree from index
Created commit objects
Linked commits using parent pointer
Updated HEAD and branch reference

Screenshots:

pes log
find .pes -type f
HEAD and refs
Commit History

The project follows incremental development with multiple commits across phases showing implementation progress.

Analysis Questions
Checkout Implementation

To implement pes checkout <branch>:

Update HEAD to point to new branch
Read commit hash from refs
Load tree from commit
Update working directory
Dirty Working Directory
Compare working directory with index
Prevent checkout if changes exist
Avoid overwriting user data
Detached HEAD
HEAD points to commit instead of branch
New commits are not linked
Can recover by creating new branch
Garbage Collection
Traverse reachable commits
Mark used objects
Delete unused objects
Race Condition
GC may delete objects during commit
Leads to inconsistency
Avoided using locking mechanisms
Build & Run

make
./pes init
./pes add file.txt
./pes commit -m "message"
./pes log

Conclusion

This project demonstrates:

Content-addressable storage
Efficient snapshot management
Basic version control system design
