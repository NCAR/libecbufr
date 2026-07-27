
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

// Redefine in case this helps avoid a symbol collision somewhere.  It
// would also allow this to be used on all platforms and not just
// UCRT.
#define alphasort alphasort_ucrt
#define scandir scandir_ucrt


int alphasort(const struct dirent **a, const struct dirent **b) {
    return strcoll((*a)->d_name, (*b)->d_name);
}

int scandir(const char *dirp,
            struct dirent ***namelist,
            int (*filter)(const struct dirent *),
            int (*compar)(const struct dirent **, const struct dirent **))
{
    DIR *dir = opendir(dirp);
    if (!dir) return -1;

    struct dirent **list = NULL;
    size_t count = 0, cap = 0;
    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL) {
        if (filter && !filter(entry))
            continue;

        if (count == cap) {
            size_t newcap = cap ? cap * 2 : 16;
            struct dirent **tmp = realloc(list, newcap * sizeof(*list));
            if (!tmp) {
                for (size_t i = 0; i < count; i++) free(list[i]);
                free(list);
                closedir(dir);
                return -1;
            }
            list = tmp;
            cap = newcap;
        }

        /* d_name is a flexible/fixed array inside struct dirent, so
           allocate a full copy of the struct (safe with mingw-w64's
           layout, which uses a fixed-size d_name buffer). */
        struct dirent *copy = malloc(sizeof(*copy));
        if (!copy) {
            for (size_t i = 0; i < count; i++) free(list[i]);
            free(list);
            closedir(dir);
            return -1;
        }
        memcpy(copy, entry, sizeof(*copy));
        list[count++] = copy;
    }

    closedir(dir);

    if (compar)
        qsort(list, count, sizeof(*list),
              (int (*)(const void *, const void *))compar);

    *namelist = list;
    return (int)count;
}

