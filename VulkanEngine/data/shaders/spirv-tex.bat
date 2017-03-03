@echo off
glslangValidator -V tex.vert -o tex.vert.spv
glslangValidator -V tex.frag -o tex.frag.spv
pause