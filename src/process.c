#include "process.h"
#include "helper.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char input[1024];

// baca seluruh isi file jadi string
static char *slurp_file(const char *path) {
  FILE *f = fopen(path, "rb");
  if (!f)
    return NULL;
  fseek(f, 0, SEEK_END);
  long sz = ftell(f);
  rewind(f);
  char *buf = malloc(sz + 1);
  if (!buf) {
    fclose(f);
    return NULL;
  }
  fread(buf, 1, sz, f);
  buf[sz] = '\0';
  fclose(f);
  return buf;
}

// skip spasi
static void skip_spaces(const char **p) {
  while (isspace((unsigned char)**p))
    (*p)++;
}

static char *parse_string(const char **p) {
  skip_spaces(p);
  if (**p != '"')
    return NULL;
  (*p)++;
  char *out = malloc(strlen(*p) + 1);
  if (!out)
    return NULL;
  int i = 0;
  while (**p && **p != '"') {
    if (**p == '\\') {
      (*p)++;
      if (**p == 'n')
        out[i++] = '\n';
      else if (**p == 't')
        out[i++] = '\t';
      else
        out[i++] = **p;
    } else {
      out[i++] = **p;
    }
    (*p)++;
  }
  if (**p == '"')
    (*p)++;
  out[i] = '\0';
  return out;
}

void start(char *path) {
  if (!isFileExists(path) || !isRightType(path)) {
    printf("Failed to proceed\n");
    return;
  }

  char *src = slurp_file(path);
  if (!src) {
    printf("Could not read file\n");
    return;
  }

  const char *p = src;
  skip_spaces(&p);

  // MAKE PRINT LIKE FUNCTION
  p += 3;
  skip_spaces(&p);
  if (*p == ':') {
    p++;
    skip_spaces(&p);
  }

  char *s = parse_string(&p);
  if (!s) {
    printf("Parse error: expected string\n");
    free(src);
    return;
  }

  printf("%s", s);
  // DONE
  skip_spaces(&p);

  // Make cin(C++) like function
  p += 2;
  skip_spaces(&p);
  if (*p == ':') {
    p++;
    skip_spaces(&p);
  }
  fgets(input, sizeof(input), stdin);
  if (!input) {
    printf("Parse error: expected input\n");
    free(src);
    return;
  }
  printf("%s", &input);

  // CLEAN & CLEAR
  free(s);
  free(src);
}
