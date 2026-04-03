#include <stdio.h>
#include <stdint.h>
#include <sys/stat.h>
#include "sha1.h"
#include "utils.h"
#include <winsock2.h>

#define COMMAND_OK 0
#define COMMAND_NOT_OK 1

typedef struct Header  // We need to extract the header
{
    char signature[4];  // "DIRC"
    uint32_t version;  // Format version (we use the version 2)
    uint32_t entries;  // Total number of file added
}Header;

typedef struct fileInfo  // Will contain file information to be store inside of index file and finally be hashed to sign
{
    uint32_t ctime_sec;  // Creation (Seconds)
    uint32_t ctime_nsec;  // Creation (nanoseconds)
    uint32_t mtime_sec;  // Modified (seconds)
    uint32_t mtime_nsec;  // Modified (nanoseconds)
    uint32_t dev;        // Devide ID
    uint32_t ino;    // inode number
    uint32_t mode;   // permissions
    uint32_t uid;  // User id
    uint32_t gid;  // Group id
    uint32_t size;   // Size of file
    unsigned char sha1[20];  // 20 byte of my sha1
    uint16_t flags;  // length of the name file
}fileInfo;

int extract_header(Header *header, char *path)
{
    if (file_exists(path) == 1)  // If the file exists
    {
        FILE *file = fopen(path, "rb");  // We open it
        fread(header, 12, 1, file);  // We read the header
        fclose(file);
        return COMMAND_OK;
    }
    fprintf(stderr, "Error: impossible to open index file.\n");
    return COMMAND_NOT_OK;
}

char *create_tree_string(char *type, size_t *size, char *tree)
{
    char buffer[256];
    int len_header = sprintf(buffer, "%s %zu", type, *size);
    char *result = allocateString(len_header + *size + 1);

    if(!result) return NULL;

    memcpy(result, buffer, len_header);

    result[len_header] = '\0';
    memcpy(result + len_header + 1, tree, *size);

    *size += len_header + 1;
    return result;
}

int create_tree()
{
    Header header;
    fileInfo fileInfo;

    if (extract_header(&header, ".ironGit/index") == COMMAND_NOT_OK) return COMMAND_NOT_OK;
    uint32_t numFile = ntohl(header.entries);

    FILE *file = fopen(".ironGit/index", "rb");  // We open it
    fseek(file, 12, SEEK_SET);  // Let's skip the header

    size_t treeSize = 0;
    char *tree = malloc(1);

    for (uint32_t i = 0; i < numFile; i++)
    {
        fread(&fileInfo, 62, 1, file);
        uint16_t len = ntohs(fileInfo.flags) & 0x0FFF;  // We reconvert the flags and mask them (first 4 bits are for ironGit)

        char *fileName = malloc(len + 1);  // The file name
        fread(fileName, len, 1, file);  // We read the file name from the file
        fileName[len] = '\0';  // Close the string

        reallocString(&tree, treeSize + len + 1 + 7 + 20);  // Realloc new len
        int written = sprintf(tree + treeSize, "%lo %s", ntohl(fileInfo.mode), fileName);
        treeSize += written;

        tree[treeSize] = '\0';
        treeSize++;

        memcpy(tree + treeSize, fileInfo.sha1, 20);
        treeSize += 20;

        uint32_t padding = 8 - ((62 + len) % 8);
        fseek(file, (int)padding, SEEK_CUR); // Skip the padding
        free(fileName);
    }
    fclose(file);
    char *newString = create_tree_string("tree", &treeSize, tree); // Creating tree <size>\0 + data
    char *hashed_string = SHA_1(newString, treeSize, NULL);  // Hashing everything
    compress_and_save((unsigned char*)newString, treeSize, hashed_string);  // Compressing and saving
    printf("Tree creato con hash: %s\n", hashed_string);
    free(newString);
    free(hashed_string);
    return COMMAND_OK;
}