#ifndef STDGEO_PARSER_H
#define STDGEO_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file stdgeo_parser.h
 * @brief C FFI interface for the stdgeo-parser library
 * 
 * This header provides a C-compatible interface to the stdgeo-parser library
 * for use in C++ GUI applications.
 */

/// Opaque handle to a parser instance
typedef struct ParserHandle ParserHandle;

/// Result of a command execution
typedef struct {
    /// Status: 0 = success, 1 = error, 2 = no output
    int status;
    /// Message (must be freed with stdgeo_parser_free_string if not NULL)
    char* message;
} CommandResultC;

/**
 * @brief Create a new parser instance
 * @return Pointer to parser handle, or NULL on failure
 */
ParserHandle* stdgeo_parser_create(void);

/**
 * @brief Destroy a parser instance
 * @param handle Parser handle to destroy
 */
void stdgeo_parser_destroy(ParserHandle* handle);

/**
 * @brief Execute a command string
 * @param handle Parser handle
 * @param command Null-terminated command string
 * @return Command result (message must be freed if not NULL)
 */
CommandResultC stdgeo_parser_execute(ParserHandle* handle, const char* command);

/**
 * @brief Get the number of geometries in the context
 * @param handle Parser handle
 * @return Number of geometries, or -1 on error
 */
int stdgeo_parser_geometry_count(ParserHandle* handle);

/**
 * @brief Clear all geometries from the context
 * @param handle Parser handle
 * @return 0 on success, -1 on error
 */
int stdgeo_parser_clear_context(ParserHandle* handle);

/**
 * @brief Free a string allocated by this library
 * @param s String to free
 */
void stdgeo_parser_free_string(char* s);

#ifdef __cplusplus
}
#endif

#endif // STDGEO_PARSER_H