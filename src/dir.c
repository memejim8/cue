#include "dir.h"

int tryOpen(const char* path) 
{
    DIR* dir = opendir(path);
    if (dir != NULL) {
        closedir(dir);
        return 1;
    }
    return 0;
}

int isDirectory(const char* path) 
{
    struct stat path_stat;
    if (stat(path, &path_stat) != 0) {
        return 0;  // Unable to get file information, assume not a directory
    }

    return S_ISDIR(path_stat.st_mode);
}

// Function to traverse a directory tree and search for a given file or directory
int walker(const char *startPath, const char *searching, char *result,
           const char *allowedExtensions, enum SearchType searchType) 
{
    DIR *d;
    struct dirent *dir;
    struct stat file_stat;
    char ext[6]; // +1 for null-terminator

    regex_t regex;
    int ret = regcomp(&regex, allowedExtensions, REG_EXTENDED);
    if (ret != 0) {
        return -1;
    }

    bool copyresult = false;

    if (startPath != NULL) {
        d = opendir(startPath);
        if (d == NULL) {
            fprintf(stderr, "Failed to open directory: %s\n", startPath);
            return 1;
        }
        chdir(startPath);
    } else {
        d = opendir(".");
        if (d == NULL) {
            fprintf(stderr, "Failed to open current directory.\n");
            return 1;
        }
    }

    while ((dir = readdir(d))) {
        if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0) {
            continue;
        }

        char entryPath[PATH_MAX];
        snprintf(entryPath, sizeof(entryPath), "%s/%s", getcwd(NULL, 0), dir->d_name);

        if (stat(entryPath, &file_stat) != 0) {
            continue;
        }

        if (S_ISDIR(file_stat.st_mode)) {
            if ((strcasestr(dir->d_name, searching) != NULL) && (searchType != FileOnly)) {
                snprintf(result, MAXPATHLEN, "%s/%s", getcwd(NULL, 0), dir->d_name);
                copyresult = true;
                break;
            } else {          
                if (chdir(dir->d_name) == -1) {
                    fprintf(stderr, "Failed to change directory: %s\n", dir->d_name);
                    continue;
                }
                if (walker(NULL, searching, result, allowedExtensions, searchType) == 0) {
                    copyresult = true;
                    break;
                }
                if (chdir("..") == -1) {
                    fprintf(stderr, "Failed to change directory to parent.\n");
                    break;
                }
            }
        } else {
            if (searchType == DirOnly) {
                continue;
            }

            char *filename = dir->d_name;
            if (strlen(filename) <= 4) {
                continue;
            }

            extractExtension(filename, sizeof(ext) - 1, ext);
            if (match_regex(&regex, ext) != 0) {
                continue;
            }

            if (strcasestr(dir->d_name, searching) != NULL) {
                snprintf(result, MAXPATHLEN, "%s/%s", getcwd(NULL, 0), dir->d_name);
                copyresult = true;
                break;
            }
        }
    }

    closedir(d);
    return copyresult ? 0 : 1;
}