#pragma once
#ifndef SITE_GEN_H
#define SITE_GEN_H

#include "common.h"
#include "slice.h"
#include "arena.h"
#include "sc_file.h"
#include "sc_to_html.h"

#define SITE_NAVIGATION_MAX_ENTRIES 32
#define SITE_BLOG_MAX_ENTRIES 4096

int GenerateSite(const char *in_dir_relative,
                 const char *out_dir_relative,
                 Arena *arena, 
                 Slice *error);

#ifndef NDEBUG
void TEST_GetSCInfo(void);
void TEST_GenerateNormalPage(void);
#endif

#endif


