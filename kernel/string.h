#ifndef STRING_H
#define STRING_H

/*
 * Compare memory areas.
 * Parameters:
 *  - v1: Pointer to the first memory area.
 *  - v2: Pointer to the second memory area.
 *  - n: Number of bytes to compare.
 * Returns:
 *  - An integer less than, equal to, or greater than zero if the first n bytes of v1 is found, respectively, to be less than, to match, or be greater than the first n bytes of v2.
 */
int memcmp(const void *v1, const void *v2, uint n);

/*
 * Move block of memory.
 * Parameters:
 *  - dst: Pointer to the destination array where the content is to be copied to.
 *  - src: Pointer to the source of data to be copied.
 *  - n: Number of bytes to copy.
 * Returns:
 *  - A pointer to the destination array dst.
 */
void* memmove(void *dst, const void *src, uint n);

/*
 * Fill block of memory.
 * Parameters:
 *  - dst: Pointer to the block of memory to fill.
 *  - c: Value to be set. The value is passed as an int, but the function fills the block of memory using the unsigned char conversion of this value.
 *  - n: Number of bytes to be set to the value.
 * Returns:
 *  - A pointer to the block of memory dst.
 */
void* memset(void *dst, int c, uint n);

/*
 * Copy strings safely by ensuring that it always adds a null terminator.
 * Parameters:
 *  - s: Destination where the content is to be copied.
 *  - t: String to be copied.
 *  - n: Maximum number of characters to be copied from source.
 * Returns:
 *  - A pointer to the destination string s.
 */
char* safestrcpy(char *s, const char *t, int n);

/*
 * Calculate the length of a string.
 * Parameters:
 *  - s: The string to be sized.
 * Returns:
 *  - The length of the string.
 */
int strlen(const char *s);

/*
 * Compare two strings.
 * Parameters:
 *  - p: The first string to be compared.
 *  - q: The second string to be compared.
 *  - n: Maximum number of characters to compare.
 * Returns:
 *  - An integer less than, equal to, or greater than zero if the first n characters of p is found, respectively, to be less than, to match, or be greater than the first n characters of q.
 */
int strncmp(const char *p, const char *q, uint n);

/*
 * Copy string.
 * Parameters:
 *  - s: Destination where the content is to be copied.
 *  - t: String to be copied.
 *  - n: Maximum number of characters to be copied from the source.
 * Returns:
 *  - A pointer to the destination string s.
 */
char* strncpy(char *s, const char *t, int n);

/*
 * Compare two strings.
 * Parameters:
 *  - p: The first string to be compared.
 *  - q: The second string to be compared.
 * Returns:
 *  - An integer less than, equal to, or greater than zero if the first character of p is found, respectively, to be less than, to match, or be greater than the first character of q.
 */
int strcmp(const char *p, const char *q);

/*
 * Convert the string to an integer.
 */
int atoi(const char *s);

#endif
