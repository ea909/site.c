#include "paths.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

// IMPORTANT(eric): The varargs for this function are null terminated!
// EX: MakePath(arena, "foo", "bar", 0);
// Don't forget it!
// Constructs a path out of a collection of sub-paths / parts.
// Arguments are a series of c-strs followed by 0 (null pointer terminator)
// For example (using unix paths):
//
// MakePath(arena, "/usr/", "local", "bin", 0) 
// returns "/usr/local/bin"
#ifdef _WIN32
#   define SC_PATH_SEP '\\'
#else
#   define SC_PATH_SEP '/'
#endif
const char *MakePath(Arena *arena, ...) {
    va_list ap;
    va_start(ap, arena);
    ArenaString path = ArenaBeginString(arena);

    char * str = va_arg(ap, char*);
    while (str) {
        ArenaPushCStr(arena, str);

        str = va_arg(ap, char*);

        // Need to add path separators
        if (str && arena->current[-1] != SC_PATH_SEP) {
            ArenaPushChar(arena, SC_PATH_SEP);
        }
    }

    ArenaPushChar(arena, 0);
    va_end(ap);

    return (const char *)path;
}

#ifdef _WIN32
#   include <windows.h>

// A DirIter is an iterator over the contents of a directory.
struct DirIter {
    char               buf[BUF_SIZE];
    HANDLE             find_handle;
    WIN32_FIND_DATAA   find_data;
};

// Sets up a directory iterator, which provides an
// iterator to access each item in the given path.
//
// This does not get the first item, it just sets things up.
// To get the first item, call GetNextItem
//
// ex:
/*
 * BeginDirIter(dir_iter, "folder");
 * while (GetNextFile(dir_iter) {
 *     ... do stuff with dir entry ...
 *  }
 *  EndDirIter(dir_iter);
 */
void BeginDirIter(DirIter *dir, const char *path) {
    memset(dir, 0, sizeof(*dir));
    memsize path_len = strlen(path);

    if (path[path_len-1] == '\\') {
        snprintf(dir->buf, BUF_SIZE, "%s*", path);
    } else {
        snprintf(dir->buf, BUF_SIZE, "%s\\*", path);
    }
}

// Get the next item in the directory
// Returns false if there is no next item, otherwise true
// Use IsDirectory, GetFileName to query the dir entry
int GetNextFile(DirIter *dir) {
    if (dir->find_handle == 0) {
        dir->find_handle = FindFirstFileA(dir->buf, &dir->find_data);
        return dir->find_handle != INVALID_HANDLE_VALUE;
    } else {
        return FindNextFileA(dir->find_handle, &dir->find_data);
    }
}

// True if current directory item is a directory
int IsDirectory(DirIter *dir) {
    return dir->find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
}

// Returns the file name of the current directory item.
// This string is stored in a buffer in the DirIter. As a result,
// GetNextFile or EndDirIter will invalidate this pointer.
const char *GetFileName(DirIter *dir) {
    return dir->find_data.cFileName;
}

// Make sure to call this to not leak HANDLEs
void EndDirIter(DirIter *dir) {
    FindClose(dir->find_handle);
}

// Change directory to the given path
// Can be relative or absolute.
// Returns false on failure.
int ChangeDirectory(const char *path) {
    return SetCurrentDirectoryA(path);
}

// Copy the current directories absolute path into the given buffer
// Null-terminated.
void CurrentDirectory(char *buf, memsize buf_len) {
    GetCurrentDirectoryA((DWORD)buf_len, buf);
}

// Makes a directory. 
// Consider successful if the directory already exists
// Returns false on failure
int MakeDirectory(const char *path) {
    if (!CreateDirectoryA(path, NULL)) {
        DWORD error = GetLastError();
        if (error != ERROR_ALREADY_EXISTS) {
            return 0;
        }
    }

    return 1;
}

// Read whole file into arena, Slice *out points to file data
// Returns false on failure
int ReadEntireFile(const char *file_path, Arena *arena, Slice *out) {
    HANDLE file = CreateFileA(file_path, 
                              GENERIC_READ,
                              0, // share mode
                              0, // security
                              OPEN_EXISTING,
                              0, // file attr
                              0); // template

    if (file == INVALID_HANDLE_VALUE) { return 0; }

    LARGE_INTEGER val;
    memset(&val, 0, sizeof(val));

    if (!GetFileSizeEx(file, &val) || val.HighPart) { 
        CloseHandle(file);
        return 0; 
    }

    ArenaPos pos = ArenaSave(arena);
    char *buf = RawArenaPush(arena, val.LowPart);
    unsigned long bytes_read = 0;

    if (!ReadFile(file, buf, val.LowPart, &bytes_read, 0) || 
        bytes_read != val.LowPart) {
        CloseHandle(file);
        ArenaRestore(arena, pos);
        return 0;
    }

    *out = MakeSlice(buf, val.LowPart);
    CloseHandle(file);
    return 1;
}

// Write whole file from slice data.
// Returns false on failure
int WriteEntireFile(Slice data, const char *file_path) {
    HANDLE file = CreateFileA(file_path, 
                              GENERIC_WRITE,
                              0, // share mode
                              0, // security
                              CREATE_ALWAYS,
                              0, // file attr
                              0); // template

    if (file == INVALID_HANDLE_VALUE) { return 0; }
    memsize len = SliceLength(data);
    unsigned long bytes_read = 0;

    if (!WriteFile(file, data.begin, (DWORD)len, &bytes_read, 0) ||
        bytes_read != len) {
        CloseHandle(file);
        return 0;
    }

    CloseHandle(file);
    return 1;
}

// I still cannot find a good way to do these.
// There is SHFileOperationA, but it is apparently deprecated and 
// replaced by the IFileOperation COM object.

void CopyDirectory(const char *src_path, const char *src_name,
                   const char *dst_path, const char *dst_name, 
                   Arena *arena) {
    const char *full_src = MakePath(arena, src_path, src_name, 0);
    const char *full_dst = MakePath(arena, dst_path, dst_name, 0);
    const char * cmd = ArenaPrintfCStr(arena,
                           "xcopy /Y /I /Q /E \"%s\" \"%s\"",
                           full_src,
                           full_dst);
    system(cmd);
}

void CopyFileToDir(const char *src_path, const char *src_name,
              const char *dst_path, Arena *arena) {
    const char *full_src = MakePath(arena, src_path, src_name, 0);
    const char *full_dst = MakePath(arena, dst_path, src_name, 0);
    const char * cmd = ArenaPrintfCStr(arena,
                           "copy /Y \"%s\" \"%s\"",
                           full_src,
                           full_dst);
    system(cmd);
}

#else // Linux/Unix/macOS/POSIX
#   include <dirent.h>
#   include <unistd.h>
#   include <sys/stat.h>
// See windows impls for commentary
//
struct DirIter {
    char buf[BUF_SIZE];
    DIR *dirp;
    struct dirent *dirent;
} DirIter;

void BeginDirIter(DirIter *dir, const char *path) {
    memset(dir, 0, sizeof(*dir));
    memsize path_len = strlen(path);
    snprintf(dir->buf, BUF_SIZE, "%s", path);
}

int GetNextFile(DirIter *dir) {
    if (!dir->dirp) {
        dir->dirp = opendir(dir->buf);

        if (!dir->dirp) {
            return 0;
        }
    }

    dir->dirent = readdir(dir->dirp);
    if (!dir->dirent) { return 0; }
    return 1;
}

int IsDirectory(DirIter *dir) {
    return dir->dirent->d_type & DT_DIR;
}

const char *GetFileName(DirIter *dir) {
    return dir->dirent->d_name;
}

void EndDirIter(DirIter *dir) {
    closedir(dir->dirp);
}

int ChangeDirectory(const char *path) {
    return chdir(path) == 0;
}

void CurrentDirectory(char *buf, memsize buf_len) {
    getcwd(buf, buf_len);
}

int MakeDirectory(const char *path) {
    return 0 == mkdir(path, 0777);
}

int ReadEntireFile(const char *file_path, Arena *arena, Slice *out) {
    FILE *file = fopen(file_path, "rb");
    if (!file) { return 0; }

    int start = ftell(file);
    if (start < 0) { goto failure; }
    if (fseek(file, 0, SEEK_END) < 0) { goto failure; }
    int end = ftell(file);
    if (end < 0) { goto failure; }
    if (fseek(file, 0, SEEK_SET) < 0) { goto failure; }
    memsize size  = (memsize)(end - start);

    char *buffer = ArenaPushMany(arena, char, size);

    memsize amt_read = fread(buffer, sizeof(char), size, file);

    if (amt_read != size) {
        goto failure;
    }

    fclose(file);
    *out = MakeSlice(buffer, size);
    return 1;

failure:
    fclose(file);
    return 0;
}

int WriteEntireFile(Slice data, const char *file_path) {
    FILE *file = fopen(file_path, "wb");
    if (!file) { 
        return 0; 
    }

    memsize size = SliceLength(data);
    memsize amt_written = fwrite(data.begin, sizeof(char), size, file);

    if (amt_written != size) {
        printf("FUCK\n");
        fclose(file);
        return 0;
    }

    fclose(file);
    return 1;
}

// meh
void CopyDirectory(const char *src_path, const char *src_name,
                   const char *dst_path, const char *dst_name, 
                   Arena *arena) {
    Arena a = *arena;
    const char *full_src = MakePath(arena, src_path, src_name, 0);
    const char *full_dst = MakePath(arena, dst_path, dst_name, 0);
    const char * cmd = ArenaPrintfCStr(arena,
                           "cp -R -T \"%s\" \"%s\"",
                           full_src,
                           full_dst);
    system(cmd);
}

void CopyFileToDir(const char *src_path, const char *src_name,
              const char *dst_path, Arena *arena) {
    Arena a = *arena;
    const char *full_src = MakePath(arena, src_path, src_name, 0);
    const char *full_dst = MakePath(arena, dst_path, src_name, 0);
    const char * cmd = ArenaPrintfCStr(arena,
                           "cp \"%s\" \"%s\"",
                           full_src,
                           full_dst);
    system(cmd);
}

#endif

DirIter *ArenaPushDirIter(Arena *a) {
    return ArenaPush(a, DirIter);
}

