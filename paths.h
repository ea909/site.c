#pragma once
#ifndef PATHS_H
#define PATHS_H
#include "common.h"
#include "arena.h"
#include "slice.h"

// IMPORTANT(eric): The varargs for this function are null terminated!
// EX: MakePath(arena, "foo", "bar", 0);
// Don't forget it!
// Constructs a path out of a collection of sub-paths / parts.
// Arguments are a series of c-strs followed by 0 (null pointer terminator)
// For example (using unix paths):
//
// MakePath(arena, "/usr/", "local", "bin", 0) 
// returns "/usr/local/bin"
//
// In retrospect, this api was really annoying. 
// C should have better varargs support... 
// (and C++'s std::initializer_list just isn't quite what
//  I want either)
const char *MakePath(Arena *arena, ...); 

// A DirIter is an iterator over the contents of a directory.
struct DirIter;
typedef struct DirIter DirIter;

// Allocates a DirIter for you
// Size varies by platorm, so this must remain opaque
DirIter *ArenaPushDirIter(Arena *a);

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
void BeginDirIter(DirIter *dir, const char *path);

// Get the next item in the directory
// Returns false if there is no next item, otherwise true
// Use IsDirectory, GetFileName to query the dir entry
int GetNextFile(DirIter *dir);

// True if current directory item is a directory
int IsDirectory(DirIter *dir);

// Returns the file name of the current directory item.
// This string is stored in a buffer in the DirIter. As a result,
// GetNextFile or EndDirIter will invalidate this pointer.
const char *GetFileName(DirIter *dir);

// Make sure to call this to not leak HANDLEs
void EndDirIter(DirIter *dir);

// Change directory to the given path
// Can be relative or absolute.
// Returns false on failure.
int ChangeDirectory(const char *path);

// Copy the current directories absolute path into the given buffer
// Null-terminated.
void CurrentDirectory(char *buf, memsize buf_len);

// Makes a directory. 
// Consider successful if the directory already exists
// Returns false on failure
int MakeDirectory(const char *path);

// Read whole file into arena, Slice *out points to file data
// Returns false on failure
int ReadEntireFile(const char *file_path, Arena *arena, Slice *out);

// Write whole file from slice data.
// Returns false on failure
int WriteEntireFile(Slice data, const char *file_path);


// Copy the contents of directory "src_name" in directory "src_path"
// to a directory name "dst_name" in directory "dst_path"
// Uses the arena for temporary storage
void CopyDirectory(const char *src_path, const char *src_name,
                   const char *dst_path, const char *dst_name, 
                   Arena *arena);

// Copy file "src_name" from "src_path" to "dst_path"
// Users the arena for temporary storage
void CopyFileToDir(const char *src_path, const char *src_name,
              const char *dst_path, Arena *arena);

#endif
