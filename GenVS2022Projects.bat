@echo off
setlocal

if not exist Build md Build
cd Build
cmake -G "Visual Studio 17 2022" -A x64 .. -DMLGE_ENABLE_SAMPLES=ON -DMLGE_ENABLE_TESTS=ON -DCZCORE_ENABLE_TESTS=ON


