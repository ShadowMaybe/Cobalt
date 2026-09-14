#ifndef __COBALT_STRING_UTILS_H_
#define __COBALT_STRING_UTILS_H_

extern const char* AllSeparators;

int isDigit(char value);
int isValidFunctionName(char value);


char * InplaceReplaceByIndex(char* pBuffer, int* size, int startIndex, int endIndex, const char* replacement);
char * InplaceInsertByIndex(char * source, int *sourceLength, int insertPoint, const char *insertedString);

const char* cobalt_find_string(const char* pBuffer, const char* S);
char* cobalt_find_string_nc(char* pBuffer, const char* S);
int cobalt_count_string(const char* pBuffer, const char* S);
char* cobalt_resize_if_needed(char* pBuffer, int *size, int addsize);
char* cobalt_inplace_replace(char* pBuffer, int* size, const char* S, const char* D);
char* cobalt_append(char* pBuffer, int* size, const char* S);
char* cobalt_inplace_insert(char* pBuffer, const char* S, char* master, int* size);
char* cobalt_getline(char* pBuffer, int num);
int cobalt_countline(const char* pBuffer);
int cobalt_getline_for(const char* pBuffer, const char* S); // get the line number for 1st occurent of S in pBuffer
char* cobalt_str_next(char *pBuffer, const char* S); // mostly as strstr, but go after the substring if found

//"blank" (space, tab, cr, lf,":", ",", ";", ".", "/")
char* cobalt_next_str(char* pBuffer);   // go to next non "blank"
char* cobalt_prev_str(char* Str, char* pBuffer);    // go to previous non "blank"
char* cobalt_next_blank(char* pBuffer);   // go to next "blank"
char* cobalt_next_line(char* pBuffer);   // go to next new line (crlf not included)

const char* cobalt_get_next_str(char* pBuffer); // get a (static) copy of next str (until next separator), can be a simple number or separator also

// those function don't try to be smart with separators...
int cobalt_countstring_simple(char* pBuffer, const char* S);
char* cobalt_inplace_replace_simple(char* pBuffer, int* size, const char* S, const char* D);


#endif // __COBALT_STRING_UTILS_H_
