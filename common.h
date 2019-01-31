#pragma once
#ifndef COMMON_H
#define COMMON_H

#define VERSION_STRING "0.1"

#include <stdint.h>
#include <stddef.h>
typedef size_t   memsize;

#define ArrayCount(arr) (sizeof(arr)/sizeof(arr[0]))

// Main arena size for loading and writing files
#define ARENA_SIZE     (128 * 1024 * 1024) 
#define MIN_ARENA_SIZE (16 * 1024 * 1024)

// Used for paths, since they have to be null
// terminated thanks to the OS APIs
#define BUF_SIZE    4096 

#endif
