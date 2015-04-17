#include <unistd.h>

#include "nmplib.h"

#include "nmp_proxy_log.h"
#include "nmp_proxy_rw_file.h"



int read_file(char **buffer, const char *file_path)
{
    FILE *fpr;
    int file_len = 0;
    NMP_ASSERT(file_path);

    fpr = fopen(file_path, "r");
    if (fpr)
    {
        /* Get file length. */
        fseek(fpr, 0, SEEK_END);
        file_len = ftell(fpr);
        fseek(fpr, 0, SEEK_SET);

        if (0 < file_len)
        {
            *buffer = (char*)nmp_new0(char, ++file_len);
            fread(*buffer, file_len-1, 1, fpr);

            fclose(fpr);
        }
    }
    else
        show_warn("open fr error [%s]\n", file_path);

    return file_len;
}

int write_file(char *buffer, size_t size, const char *file_path)
{
    FILE *fpw;
    NMP_ASSERT(buffer && 0<size && file_path);

    fpw = fopen(file_path, "w");
    if (fpw)
    {
        size = fwrite(buffer, size, 1, fpw);
        fsync((int)fpw);
        fclose(fpw);
    }
    else
    {
        show_warn("open fw error [%s]\n", file_path);
        return -1;
    }

    return size;
}


