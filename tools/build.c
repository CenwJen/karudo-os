#if defined(__MINGW32__) && !defined(__USE_MINGW_ANSI_STDIO)
#define __USE_MINGW_ANSI_STDIO 1
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

enum {
    SECTOR_SIZE = 512,
    SETUP_SECTORS = 4,
    SETUP_SIZE = SECTOR_SIZE * SETUP_SECTORS
};

static int read_module(const char *path, unsigned char *data, size_t limit,
                       size_t *size)
{
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Cannot open %s: %s\n", path, strerror(errno));
        return 0;
    }

    *size = fread(data, 1, limit + 1, file);
    if (ferror(file)) {
        fprintf(stderr, "Cannot read %s: %s\n", path, strerror(errno));
        fclose(file);
        return 0;
    }
    if (fclose(file) != 0) {
        fprintf(stderr, "Cannot close %s: %s\n", path, strerror(errno));
        return 0;
    }
    if (*size == 0 || *size > limit) {
        fprintf(stderr, "%s must contain 1 to %zu bytes.\n", path, limit);
        return 0;
    }
    return 1;
}

int main(int argc, char **argv)
{
    unsigned char bootsect[SECTOR_SIZE + 1] = {0};
    unsigned char setup[SETUP_SIZE + 1] = {0};
    size_t bootsect_size;
    size_t setup_size;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s bootsect setup > linux.img\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (!read_module(argv[1], bootsect, SECTOR_SIZE, &bootsect_size) ||
        !read_module(argv[2], setup, SETUP_SIZE, &setup_size)) {
        return EXIT_FAILURE;
    }
    if (bootsect_size != SECTOR_SIZE) {
        fprintf(stderr, "bootsect must be exactly %d bytes.\n", SECTOR_SIZE);
        return EXIT_FAILURE;
    }
    if (bootsect[510] != 0x55 || bootsect[511] != 0xaa) {
        fprintf(stderr, "bootsect must end with the signature 55 AA.\n");
        return EXIT_FAILURE;
    }

#ifdef _WIN32
    if (_setmode(_fileno(stdout), _O_BINARY) == -1) {
        fprintf(stderr, "Cannot use binary output: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }
#endif

    if (fwrite(bootsect, 1, SECTOR_SIZE, stdout) != SECTOR_SIZE ||
        fwrite(setup, 1, SETUP_SIZE, stdout) != SETUP_SIZE ||
        fflush(stdout) != 0) {
        fprintf(stderr, "Cannot write the complete boot image.\n");
        return EXIT_FAILURE;
    }

    fprintf(stderr, "bootsect: %zu bytes; setup: %zu bytes; image: %d bytes\n",
            bootsect_size, setup_size, SECTOR_SIZE + SETUP_SIZE);
    return EXIT_SUCCESS;
}
