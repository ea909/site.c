cmake_minimum_required (VERSION 2.6)
project(site)
add_executable(site slice.c 
                    arena.c 
                    paths.c 
                    sc_file.c 
                    sc_to_html.c 
                    site_gen.c
                    site.c)

