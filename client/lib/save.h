#ifndef SAVE_H
#define SAVE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Opens the properties file with the given mode.
 * @param mode The mode to open the file with (e.g., "r" for read, "w" for write).
 * @return A pointer to the opened file, or NULL on failure.
 */
FILE* open_properties(const char* mode);

/**
 * Reads a property from the properties file.
 * @param buf The buffer to store the property value.
 * @param property The name of the property to read.
 * @return EXIT_SUCCESS on success, or EXIT_FAILURE on error.
 */
int read_property(char* buf, const char* property);

/**
 * Writes a property to the properties file.
 * @param property The name of the property to write.
 * @param value The value of the property to write.
 * @return EXIT_SUCCESS on success, or EXIT_FAILURE on error.
 */
int write_property(const char* property, const char* value);

#endif // SAVE_H