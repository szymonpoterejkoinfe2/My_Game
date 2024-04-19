#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <filesystem>
#include <fstream>
#include <string>

#ifdef _Win32

#define DEBUG_BREAK() __debugbreak()

#endif

/// Loger

enum TextColor
{

    TEXT_COLOR_BLACK,
    TEXT_COLOR_RED,
    TEXT_COLOR_GREEN,
    TEXT_COLOR_YELLOW,
    TEXT_COLOR_BLUE,
    TEXT_COLOR_MAGENTA,
    TEXT_COLOR_CYAN,
    TEXT_COLOR_WHITE,
    TEXT_COLOR_BRIGHT_BLACK,
    TEXT_COLOR_BRIGHT_RED,
    TEXT_COLOR_BRIGHT_GREEN,
    TEXT_COLOR_BRIGHT_YELLOW,
    TEXT_COLOR_BRIGHT_BLUE,
    TEXT_COLOR_BRIGHT_MAGENTA,
    TEXT_COLOR_BRIGHT_CYAN,
    TEXT_COLOR_BRIGHT_WHITE,
    TEXT_COLOR_COUNT

};

template <typename... Args>
void _log(char *prefix, char *msg, TextColor textColor, Args... args)
{

    static char *TextColorTable[TEXT_COLOR_COUNT] =
        {

            "\x1b[30m",
            "\x1b[31m",
            "\x1b[32m",
            "\x1b[33m",
            "\x1b[34m",
            "\x1b[35m",
            "\x1b[36m",
            "\x1b[37m",
            "\x1b[90m",
            "\x1b[91m",
            "\x1b[92m",
            "\x1b[93m",
            "\x1b[94m",
            "\x1b[95m",
            "\x1b[96m",
            "\x1b[97m",

        };

    char formatBuffer[8192] = {};

    sprintf(formatBuffer, "%s %s %s \033[0m", TextColorTable[textColor], prefix, msg);

    char textBuffer[8912] = {};
    sprintf(textBuffer, formatBuffer, args...);

    puts(textBuffer);
}

#define SM_TRACE(msg, ...) _log("TRACE: ", msg, TEXT_COLOR_GREEN, ##__VA_ARGS__);
#define SM_WARN(msg, ...) _log("WARN: ", msg, TEXT_COLOR_YELLOW, ##__VA_ARGS__);
#define SM_ERROR(msg, ...) _log("ERROR: ", msg, TEXT_COLOR_RED, ##__VA_ARGS__);

#define SM_ASSERT(condition, msg, ...)    \
    {                                     \
        if (!condition)                   \
        {                                 \
            SM_ERROR(msg, ##__VA_ARGS__); \
            __debugbreak();               \
        }                                 \
    }

/// Bump Allocator

struct BumpAllocator
{
    size_t capacity;
    size_t used;
    char *memory;
};

BumpAllocator make_bump_allocator(size_t size)
{
    BumpAllocator ba = {};
    ba.memory = (char *)malloc(size);

    if (ba.memory)
    {
        ba.capacity = size;
        memset(ba.memory, 0, size);
    }
    else
    {
        SM_ASSERT(false, "Failed to allocate Memory!");
    }

    ba.capacity = size;

    return ba;
}

char *bump_alloc(BumpAllocator *bumpAllocator, size_t size)
{
    char *result = nullptr;

    size_t allignedSize = (size + 7) & ~7;

    if (bumpAllocator->used + allignedSize <= bumpAllocator->capacity)
    {
        result = bumpAllocator->memory + bumpAllocator->used;
        bumpAllocator->used += allignedSize;
    }
    else
    {
        SM_ASSERT(false, "BumoAllocator is full");
    }

    return result;
}

// #########
// File I/O
// ######

std::filesystem::file_time_type get_timestamp(std::filesystem::path &filePath)
{
    std::filesystem::file_time_type ftime = std::filesystem::last_write_time(filePath);

    return ftime;
}

bool file_exists(std::filesystem::path &filePath)
{
    bool exists = std::filesystem::exists(filePath);

    return exists;
}

long get_file_size(std::filesystem::path &filePath)
{

    long fileSize = 0;

    if (!file_exists(filePath))
    {
        SM_ERROR("Failed opening file: %s", filePath);
        return 0;
    }

    fileSize = std::filesystem::file_size(filePath);

    return fileSize;
}

char *read_file(char *filePath, int *fileSize, char *buffer)
{
    *fileSize = 0;

    std::filesystem::path pathConverted = filePath;
    if (!file_exists(pathConverted))
    {
        SM_ERROR("Failed opening file: %s", filePath);
        return nullptr;
    }

    auto file = fopen(filePath, "rb");

    fseek(file, 0, SEEK_END);
    *fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    memset(buffer, 0, *fileSize + 1);

    fread(buffer, sizeof(char), *fileSize, file);

    fclose(file);

    return buffer;
}

char *read_file(char *filePath, int *fileSize, BumpAllocator *bumpAllocator)
{
    char *file = nullptr;

    std::filesystem::path pathConverted = filePath;
    long fileSize2 = get_file_size(pathConverted);

    if (fileSize2)
    {
        char *buffer = bump_alloc(bumpAllocator, fileSize2 + 1);
        file = read_file(filePath, fileSize, buffer);
    }

    return file;
}

void write_file(char *filePath, char *buffer, int size)
{
    std::filesystem::path pathConverted = filePath;
    if (!file_exists(pathConverted))
    {
        SM_ERROR("Failed opening file: %s", filePath);
        return;
    }

    auto file = fopen(filePath, "wb");

    fwrite(buffer, sizeof(char), size, file);
    fclose(file);
}

bool copy_file(char *fileName, char *outputName, char *buffer)
{

    int fileSize = 0;
    char *data = read_file(fileName, &fileSize, buffer);

    auto outputFile = fopen(outputName, "wb");
    if (!outputFile)
    {
        SM_ERROR("Failed opening file: %s", outputName);
        return false;
    }

    fclose(outputFile);

    return true;
}

bool copy_file(char *fileName, char *outputName, BumpAllocator *bumpAllocator)
{

    char *file = 0;
    std::filesystem::path pathConverted = fileName;
    long fileSize2 = get_file_size(pathConverted);

    if (fileSize2)
    {
        char *buffer = bump_alloc(bumpAllocator, fileSize2 + 1);
        return copy_file(fileName, outputName, buffer);
    }

    return false;
}