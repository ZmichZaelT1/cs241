/**
 * finding_filesystems
 * CS 241 - Spring 2022
 */
#include "minixfs.h"
#include "minixfs_utils.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

/**
 * Virtual paths:
 *  Add your new virtual endpoint to minixfs_virtual_path_names
 */
char *minixfs_virtual_path_names[] = {"info", /* add your paths here*/};

/**
 * Forward declaring block_info_string so that we can attach unused on it
 * This prevents a compiler warning if you haven't used it yet.
 *
 * This function generates the info string that the virtual endpoint info should
 * emit when read
 */
static char *block_info_string(ssize_t num_used_blocks) __attribute__((unused));
static char *block_info_string(ssize_t num_used_blocks) {
    char *block_string = NULL;
    ssize_t curr_free_blocks = DATA_NUMBER - num_used_blocks;
    asprintf(&block_string,
             "Free blocks: %zd\n"
             "Used blocks: %zd\n",
             curr_free_blocks, num_used_blocks);
    return block_string;
}

// Don't modify this line unless you know what you're doing
int minixfs_virtual_path_count =
    sizeof(minixfs_virtual_path_names) / sizeof(minixfs_virtual_path_names[0]);

int minixfs_chmod(file_system *fs, char *path, int new_permissions) {
    // Thar she blows!
    inode *n = get_inode(fs, path);
    if (n) {
        n->mode = new_permissions | (n->mode >> RWX_BITS_NUMBER) << RWX_BITS_NUMBER;
        clock_gettime(CLOCK_REALTIME, &n->ctim);
        return 0;
    } else {
        errno = ENOENT;
        return -1;
    }
}

int minixfs_chown(file_system *fs, char *path, uid_t owner, gid_t group) {
    // Land ahoy!
    inode *n = get_inode(fs, path);
    if (n) {
        if (owner != ((uid_t)-1)) {
            n->uid = owner;
        } 
        if (group != ((uid_t)-1)) {
            n->gid = group;
        }
        clock_gettime(CLOCK_REALTIME, &n->ctim);
        return 0;
    } else {
        errno = ENOENT;
        return -1;
    }
}

inode *minixfs_create_inode_for_path(file_system *fs, const char *path) {
    // Land ahoy!
    if (get_inode(fs, path) || !valid_filename(path)) {
        return NULL;
    }

    const char *fileName;
    inode *parent_n = parent_directory(fs, path, &fileName);
    if (!parent_n || !is_directory(parent_n)) {
        return NULL;
    }

    inode_number new_inum = first_unused_inode(fs);
    if (new_inum == -1) {
        return NULL;
    }
    inode *first_inode = fs->inode_root + new_inum;
    init_inode(parent_n, first_inode);

    data_block_number last_block;

    size_t num_block = parent_n->size / sizeof(data_block);
    size_t offset = parent_n->size % sizeof(data_block);
    if (num_block >= NUM_DIRECT_BLOCKS && parent_n->indirect == UNASSIGNED_NODE && add_single_indirect_block(fs, parent_n) == -1) {
        return NULL;
    }

    char *block = NULL;
    if (num_block < NUM_DIRECT_BLOCKS) {
        last_block = parent_n->direct[num_block];
        block = fs->data_root[last_block].data + offset;
    } else {
        last_block = parent_n->indirect;
        size_t remaining_size = (num_block - NUM_DIRECT_BLOCKS) * sizeof(data_block_number);
        data_block_number block_num = (data_block_number)(fs->data_root[last_block].data + remaining_size);
        block = fs->data_root[block_num].data + offset;
    }

    minixfs_dirent d;
    d.name = (char*)fileName;
    d.inode_num = new_inum;
    make_string_from_dirent(block, d);

    parent_n->size += FILE_NAME_ENTRY;
    clock_gettime(CLOCK_REALTIME, &parent_n->mtim);

    return first_inode;
}

ssize_t minixfs_virtual_read(file_system *fs, const char *path, void *buf,
                             size_t count, off_t *off) {
    if (!strcmp(path, "info")) {
        // TODO implement the "info" virtual file here
        char *data_m = GET_DATA_MAP(fs->meta);
        ssize_t cnt = 0;

        for (uint64_t i = 0; i < fs->meta->dblock_count; i++) {
            if (data_m[i] == 1) {
                cnt++;
            }
        }

        char *info = block_info_string(cnt);
        if (*off > (long)strlen(info)) {
            return 0;
        }
        if (cnt > ((long)strlen(info) - *off)) {
            cnt = (long)strlen(info) - *off;
        }
        memmove(buf, info + *off, cnt);
        *off += cnt;
        return cnt;
    }

    errno = ENOENT;
    return -1;
}

ssize_t minixfs_write(file_system *fs, const char *path, const void *buf,
                      size_t count, off_t *off) {
    // X marks the spot
    inode *n = get_inode(fs, path);
    if (!n) {
        n = minixfs_create_inode_for_path(fs, path);
    }

    uint64_t total = (NUM_DIRECT_BLOCKS + NUM_INDIRECT_BLOCKS) * sizeof(data_block);

    int block_num = (count + *off + sizeof(data_block) - 1) / sizeof(data_block);
    if (minixfs_min_blockcount(fs, path, block_num) == -1 || (count + *off) > total) {
        errno = ENOSPC;
        return -1;
    }

    size_t num_block = *off / sizeof(data_block);
    size_t offset = *off % sizeof(data_block);

    size_t bytes_written = 0;
    data_block_number block_number;

    // writing
    while (bytes_written < count) {
        // if no blocks left
        if (num_block < NUM_DIRECT_BLOCKS) {
            block_number = n->direct[block_num];
        } else {
            size_t remaining_size = (num_block - NUM_DIRECT_BLOCKS) * sizeof(data_block_number);
            block_number = (data_block_number)(fs->data_root[n->indirect].data + remaining_size);
        }
        data_block block = fs->data_root[block_number];

        // if less than a block
        size_t buffer_size = fmax((sizeof(data_block) - offset), total - bytes_written);

        memcpy(block.data + offset, buf + bytes_written, buffer_size);
        bytes_written += buffer_size;
        num_block++;
        offset = 0;
    }

    *off += count;
    n->size = *off;

    //update time
    clock_gettime(CLOCK_REALTIME, &(n->atim));
    clock_gettime(CLOCK_REALTIME, &(n->mtim));

    return -1;
}

ssize_t minixfs_read(file_system *fs, const char *path, void *buf, size_t count,
                     off_t *off) {
    const char *virtual_path = is_virtual_path(path);
    if (virtual_path)
        return minixfs_virtual_read(fs, virtual_path, buf, count, off);
    // 'ere be treasure!
    inode *n = get_inode(fs, path);
    if (!n) {
        errno = ENOENT;
        return -1;
    }

    size_t num_block = *off / sizeof(data_block);
    size_t offset = *off % sizeof(data_block);

    // If *off is greater than the end of the file,
    if (*off > (long)n->size) {
        return 0;
    }

    size_t bytes_read = 0;
    data_block_number block_number;
    size_t total;

    // `min(count, size_of_file - *off)`
    if (count > n->size - *off) {
        total = n->size - *off;
    } else {
        total = count;
    }

    // reading
    while (bytes_read < total) {
        // if no blocks left
        if (num_block < NUM_DIRECT_BLOCKS) {
            block_number = n->direct[num_block];
        } else {
            size_t remaining_size = (num_block - NUM_DIRECT_BLOCKS) * sizeof(data_block_number);
            block_number = (data_block_number)(fs->data_root[n->indirect].data + remaining_size);
        }
        data_block block = fs->data_root[block_number];

        // if less than a block
        size_t buffer_size = fmax((sizeof(data_block) - offset), total - bytes_read);

        memcpy(buf + bytes_read, block.data + offset, buffer_size);
        bytes_read += buffer_size;
        num_block++;
        offset = 0;
    }

    *off += bytes_read;
    
    //update time
    clock_gettime(CLOCK_REALTIME, &(n->atim));

    return -1;
}
