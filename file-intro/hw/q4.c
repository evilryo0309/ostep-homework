#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

void search_dir(const char *dir_name, const char *target_name)
{
    DIR *dir = opendir(dir_name);
    if (dir == NULL)
    {
        // Silently skip unreadable directories (permission denied)
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        // Skip current and parent directory pointers to prevent infinite loops
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", dir_name, entry->d_name);

        // Print the path if no filter is applied, or if the filename matches the filter
        if (target_name == NULL || strcmp(entry->d_name, target_name) == 0)
        {
            printf("%s\n", path);
        }

        struct stat statbuf;
        // Use lstat instead of stat to avoid infinitely traversing symbolic links
        if (lstat(path, &statbuf) == 0 && S_ISDIR(statbuf.st_mode))
        {
            search_dir(path, target_name);
        }
    }
    closedir(dir);
}

int main(int argc, char *argv[])
{
    char *start_path = ".";
    char *target_name = NULL;

    // Parse arguments to support custom paths and a -name filter
    if (argc >= 2)
    {
        if (strcmp(argv[1], "-name") == 0 && argc >= 3)
        {
            target_name = argv[2];
        }
        else
        {
            start_path = argv[1];
            if (argc >= 4 && strcmp(argv[2], "-name") == 0)
            {
                target_name = argv[3];
            }
        }
    }

    // Print the root directory itself if we aren't filtering by name
    if (target_name == NULL)
    {
        printf("%s\n", start_path);
    }

    search_dir(start_path, target_name);

    return 0;
}