@echo off
glslangValidator -V color.vert -o color.vert.spv
glslangValidator -V color.frag -o color.frag.spv
pause