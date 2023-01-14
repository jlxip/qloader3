#include <stdint.h>
#include <stddef.h>
#include <lib/uri.h>
#include <lib/misc.h>
#include <lib/part.h>
#include <lib/libc.h>
#include <fs/file.h>
#include <mm/pmm.h>
#include <lib/print.h>
#include <tinf.h>

// A URI takes the form of: resource://root/path
// The following function splits up a URI into its componenets
bool uri_resolve(char *uri, char **resource, char **root, char **path) {
    size_t length = strlen(uri) + 1;
    char *buf = ext_mem_alloc(length);
    memcpy(buf, uri, length);
    uri = buf;

    *resource = *root = *path = NULL;

    // Get resource
    for (size_t i = 0; ; i++) {
        if (strlen(uri + i) < 3)
            return false;

        if (!memcmp(uri + i, "://", 3)) {
            *resource = uri;
            uri[i] = 0;
            uri += i + 3;
            break;
        }
    }

    // Get root
    for (size_t i = 0; ; i++) {
        if (uri[i] == 0)
            return false;

        if (uri[i] == '/') {
            *root = uri;
            uri[i] = 0;
            uri += i + 1;
            break;
        }
    }

    // Get path
    if (*uri == 0)
        return false;
    *path = uri;

    return true;
}

static bool parse_bios_partition(char *loc, int *drive, int *partition) {
    uint64_t val;

    for (size_t i = 0; ; i++) {
        if (loc[i] == 0)
            return false;

        if (loc[i] == ':') {
            loc[i] = 0;
            if (*loc == 0) {
                panic(true, "Drive number cannot be omitted for hdd:// and odd://");
            } else {
                val = strtoui(loc, NULL, 10);
                if (val < 1 || val > 256) {
                    panic(true, "Drive number outside range 1-256");
                }
                *drive = val;
            }
            loc += i + 1;
            break;
        }
    }

    val = strtoui(loc, NULL, 10);
    if (val > 256) {
        panic(true, "Partition number outside range 0-256");
    }
    *partition = val;

    return true;
}

static struct file_handle *uri_hdd_dispatch(char *loc, char *path) {
    int drive, partition;

    if (!parse_bios_partition(loc, &drive, &partition))
        return NULL;

    struct volume *volume = volume_get_by_coord(false, drive, partition);

    if (volume == NULL)
        return NULL;

    return fopen(volume, path);
}

static struct file_handle *uri_odd_dispatch(char *loc, char *path) {
    int drive, partition;

    if (!parse_bios_partition(loc, &drive, &partition))
        return NULL;

    struct volume *volume = volume_get_by_coord(true, drive, partition);

    if (volume == NULL)
        return NULL;

    return fopen(volume, path);
}

static struct file_handle *uri_guid_dispatch(char *guid_str, char *path) {
    struct guid guid;
    if (!string_to_guid_be(&guid, guid_str))
        return NULL;

    struct volume *volume = volume_get_by_guid(&guid);
    if (volume == NULL) {
        if (!string_to_guid_mixed(&guid, guid_str))
            return NULL;

        volume = volume_get_by_guid(&guid);
        if (volume == NULL)
            return NULL;
    }

    return fopen(volume, path);
}

static struct file_handle *uri_fslabel_dispatch(char *fslabel, char *path) {
    struct volume *volume = volume_get_by_fslabel(fslabel);
    if (volume == NULL) {
        return NULL;
    }

    return fopen(volume, path);
}

static struct file_handle *uri_boot_dispatch(char *s_part, char *path) {
    int partition;

    if (s_part[0] != '\0') {
        uint64_t val = strtoui(s_part, NULL, 10);
        if (val > 256) {
            panic(true, "Partition number outside range 0-256");
        }
        partition = val;
    } else {
        partition = boot_volume->partition;
    }

    struct volume *volume = volume_get_by_coord(boot_volume->is_optical,
                                                boot_volume->index, partition);
    if (volume == NULL)
        return NULL;

    return fopen(volume, path);
}

struct file_handle *uri_open(char *uri) {
    struct file_handle *ret;

    char *resource, *root, *path;
    uri_resolve(uri, &resource, &root, &path);

    if (resource == NULL) {
        panic(true, "No resource specified for URI `%s`.", uri);
    }

    bool compressed = false;
    if (*resource == '$') {
        compressed = true;
        resource++;
    }

    if (!strcmp(resource, "bios")) {
        panic(true, "bios:// resource is no longer supported. Check CONFIG.md for hdd:// and odd://");
    } else if (!strcmp(resource, "hdd")) {
        ret = uri_hdd_dispatch(root, path);
    } else if (!strcmp(resource, "odd")) {
        ret = uri_odd_dispatch(root, path);
    } else if (!strcmp(resource, "boot")) {
        ret = uri_boot_dispatch(root, path);
    } else if (!strcmp(resource, "guid")) {
        ret = uri_guid_dispatch(root, path);
    } else if (!strcmp(resource, "uuid")) {
        ret = uri_guid_dispatch(root, path);
    } else if (!strcmp(resource, "fslabel")) {
        ret = uri_fslabel_dispatch(root, path);
    } else {
        panic(true, "Resource `%s` not valid.", resource);
    }

    if (compressed && ret != NULL) {
        struct file_handle *compressed_fd = ext_mem_alloc(sizeof(struct file_handle));
        fread(ret, &compressed_fd->size, ret->size - 4, sizeof(uint32_t));
        compressed_fd->fd = ext_mem_alloc(compressed_fd->size);
        void *src = freadall(ret, MEMMAP_BOOTLOADER_RECLAIMABLE);
        if (tinf_gzip_uncompress(compressed_fd->fd, src, ret->size)) {
            panic(true, "tinf error");
        }
        compressed_fd->vol = ret->vol;
        fclose(ret);
        compressed_fd->is_memfile = true;
        ret = compressed_fd;
    }

    return ret;
}
