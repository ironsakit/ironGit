# ironGit

A lightweight, from-scratch implementation of core Git functionalities, written entirely in C. 

This project aims to recreate the foundational mechanics of Git to understand its internal version control system. It features a custom-built SHA-1 hashing algorithm, CLI argument parsing, and native `zlib` compression for object storage, mirroring how the real Git handles blobs and trees.

## Features

Currently, `irongit` supports repository initialization, raw object database manipulation, advanced file ignoring, recursively staging files to the Index, and generating Tree objects.

| Command | Description |
| :--- | :--- |
| <nobr>`irongit init`</nobr> | Initializes or reinitializes an empty Git repository in the current directory (`.ironGit/`). |
| <nobr>`irongit hash-object -w <file>`</nobr> | Hashes a file, compresses it, and writes it to the object database as a `blob`. |
| <nobr>`irongit cat-file -p <hash>`</nobr> | Pretty-prints the uncompressed content of a specific object. |
| <nobr>`irongit cat-file -s <hash>`</nobr> | Prints the size of the object in bytes. |
| <nobr>`irongit cat-file -t <hash>`</nobr> | Prints the type of the object (e.g., `blob`, `tree`). |
| <nobr>`irongit add <file>`</nobr> | Stages a single file. Generates its Blob and updates/creates the binary `.ironGit/index` file. |
| <nobr>`irongit add .`</nobr> | Recursively scans the directory tree, applies `.irongitignore` rules, sorts files alphabetically, and writes the entire `.ironGit/index` binary in one go. |
| <nobr>`irongit write-tree`</nobr> | Parses the `.ironGit/index` binary file to generate and store a `tree` object representing the current directory structure. |

## Technical Details

* **Language:** C
* **Hashing:** Uses a hand-crafted, custom SHA-1 implementation to generate 40-character hex hashes.
* **Compression:** Utilizes `zlib` at compression level 1 (`Z_BEST_SPEED`) for fast read/write operations.
* **Object Storage Format:** Objects are stored exactly like real Git. The format is `<type> <size>\0<content>`.
* **File Structure:** When an object is saved, the first 2 characters of the SHA-1 hash are used as the directory name, and the remaining 38 characters become the compressed file's name.
* **Index Architecture:** The `.ironGit/index` binary file is generated using a dynamic buffer approach (`malloc`/`realloc`) to perfectly pre-calculate the global 20-byte SHA-1 hash before writing to disk.
* **Tree Generation:** Reads the binary Index file, extracts file metadata (handling Network Byte Order conversions for flags and permissions), calculates dynamic memory padding, and constructs valid Git `tree` objects in memory using the precise `<mode> <name>\0<raw_20_byte_hash>` format before hashing and compressing.
* **Directory Traversal & Ignore System:** Implements fully recursive directory scanning. It automatically skips restricted internal directories (like `.git`) and parses `.irongitignore.txt` to safely exclude files based on exact matches or wildcard extensions (e.g., `*.exe`).
* **Sorting:** Includes a custom-built Merge Sort algorithm to order staging area entries alphabetically by filename, ensuring 100% compatibility with the original Git's index parser.

## Building the Project

### Prerequisites
You need a C compiler (like `gcc`) and the `zlib` development libraries installed on your system.

### Compilation
A `Makefile` is provided to easily compile the source code. If you are on Windows using MinGW, run the following command in the root directory:

```bash
mingw32-make
```
*(Note: On Linux or macOS environments, you can simply use `make`)*

This executes the following build command, linking the `zlib` library:
`gcc -Wall index.c main.c objects.c utils/*.c -o irongit.exe -lz -lws2_32`

## Usage Example

```bash
# Initialize the repository
$ irongit init
Reinitialized existing Git repository.

# Stage all files recursively, ignoring what's in .irongitignore.txt
$ irongit add .

# Verify compatibility with the REAL Git's Index!
$ git ls-files --stage
100644 9fb7bccf006507a7f90d438dc442e60a73cf601d 0       ./test/dwa.txt
100644 438fc5d996b802d973388a4f6a45f5dfc5242921 0       ./test/wad.txt
100644 a6f34c09cbe1eab96ff2932dc839798ec6c3f694 0       ./utils/utils.c

# Generate the Tree object from the Index
$ irongit write-tree
Tree generated with hash: df064ee989f2f01c182d8af47005d7af217cfff1

# Verify the generated Tree object with REAL Git!
$ git --git-dir=.ironGit cat-file -p df064ee989f2f01c182d8af47005d7af217cfff1
100644 blob 9fb7bccf006507a7f90d438dc442e60a73cf601d    ./test/dwa.txt
100644 blob 438fc5d996b802d973388a4f6a45f5dfc5242921    ./test/wad.txt
100644 blob a6f34c09cbe1eab96ff2932dc839798ec6c3f694    ./utils/utils.c
```

## Work In Progress 🚧

1. **The Commit Command:** Generating `commit` objects (author metadata, Unix timestamps, and messages) that link to the generated `tree` objects to complete the core Git snapshot lifecycle.
2. **Updating HEAD:** Pointing the repository's HEAD to the newly created commit hashes.
3. **Reading the History:** Implementing a basic `irongit log` to traverse the commit chain and display the repository's history.