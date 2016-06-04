@echo off
glslangValidator -V texture.vert -o texture.vert.spv
glslangValidator -V texture.frag -o texture.frag.spv
pause