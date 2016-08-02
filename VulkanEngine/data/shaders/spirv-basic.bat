@echo off
glslangValidator -V basic.vert -o basic.vert.spv
glslangValidator -V basic.frag -o basic.frag.spv
pause