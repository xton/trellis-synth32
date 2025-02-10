#pragma once

#define PRINT_SIZE_ERROR(T) char(*__kaboom)[sizeof(T)] = 1

#define DEBUG_TRACE Serial.printf("TRACE %s %d\n", __FILE__, __LINE__)