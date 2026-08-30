#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>

// Detailed output format when processing the -l option
void print_long_format(const char *dir_path, const char *file_name)
{
    char full_path[1024];
    // Construct full file path to ensure stat() can correctly find the file in different directories
    snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, file_name);

    struct stat fileStat;
    if (stat(full_path, &fileStat) < 0)
    {
        perror("stat failed");
        return;
    }

    // Print file type and permissions (rwxrwxrwx)
    printf("%c", S_ISDIR(fileStat.st_mode) ? 'd' : '-');
    printf("%c", (fileStat.st_mode & S_IRUSR) ? 'r' : '-');
    printf("%c", (fileStat.st_mode & S_IWUSR) ? 'w' : '-');
    printf("%c", (fileStat.st_mode & S_IXUSR) ? 'x' : '-');
    printf("%c", (fileStat.st_mode & S_IRGRP) ? 'r' : '-');
    printf("%c", (fileStat.st_mode & S_IWGRP) ? 'w' : '-');
    printf("%c", (fileStat.st_mode & S_IXGRP) ? 'x' : '-');
    printf("%c", (fileStat.st_mode & S_IROTH) ? 'r' : '-');
    printf("%c", (fileStat.st_mode & S_IWOTH) ? 'w' : '-');
    printf("%c", (fileStat.st_mode & S_IXOTH) ? 'x' : '-');

    // Get owner and group names
    struct passwd *pw = getpwuid(fileStat.st_uid);
    struct group *gr = getgrgid(fileStat.st_gid);

    // Print Link count, Owner, Group, Size, File name
    printf(" %2ld", fileStat.st_nlink);
    printf(" %s", pw ? pw->pw_name : "unknown");
    printf(" %s", gr ? gr->gr_name : "unknown");
    printf(" %8ld", fileStat.st_size);
    printf(" %s\n", file_name);
}

int main(int argc, char *argv[])
{
    int l_flag = 0;
    char *dir_path = NULL;

    // Simple argument parsing
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-l") == 0)
        {
            l_flag = 1;
        }
        else
        {
            dir_path = argv[i];
        }
    }

    // If no directory is specified, use getcwd() to get current working directory
    char cwd[1024];
    if (dir_path == NULL)
    {
        if (getcwd(cwd, sizeof(cwd)) != NULL)
        {
            dir_path = cwd;
        }
        else
        {
            perror("getcwd failed");
            exit(EXIT_FAILURE);
        }
    }

    // Open and read directory
    DIR *dir = opendir(dir_path);
    if (dir == NULL)
    {
        perror("opendir failed");
        exit(EXIT_FAILURE);
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (l_flag)
        {
            print_long_format(dir_path, entry->d_name);
        }
        else
        {
            printf("%s\n", entry->d_name);
        }
    }

    closedir(dir);
    return 0;
}