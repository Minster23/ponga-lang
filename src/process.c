#include "process.h"
#include "helper.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_VARS 128

struct VAR_IN {
  char name[64];
  char *strval;
  int intval;
  int boolval;
  char type[16];
  int isActive;
};

static struct VAR_IN vars[MAX_VARS];
static int var_count = 0;

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

static char *parse_word(const char **p) {
  skip_spaces(p);
  const char *start = *p;
  while (**p && (isalnum((unsigned char)**p) || **p == '_'))
    (*p)++;
  size_t len = *p - start;
  if (len == 0)
    return NULL;
  char *word = malloc(len + 1);
  strncpy(word, start, len);
  word[len] = '\0';
  return word;
}

static struct VAR_IN *find_var(const char *name) {
  for (int i = 0; i < var_count; i++) {
    if (strcmp(vars[i].name, name) == 0)
      return &vars[i];
  }
  return NULL;
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

  while (*p) {
    skip_spaces(&p);
    if (*p == '\0')
      break;

    if (strncmp(p, "out", 3) == 0) {
      p += 3;
      skip_spaces(&p);
      if (*p == ':') {
        p++;
        skip_spaces(&p);
      }
      char *s = parse_string(&p);
      if (s) {
        printf("%s", s);
        free(s);
      } else {
        char *name = parse_word(&p);
        if (name) {
          struct VAR_IN *v = find_var(name);
          if (v) {
            if (strcmp(v->type, "string") == 0 && v->strval) {
              printf("%s", v->strval);
            } else if (strcmp(v->type, "bool") == 0) {
              printf("%s", v->boolval ? "true" : "false");
            } else if (strcmp(v->type, "int") == 0) {
              printf("%d", v->intval);
            }
          } else {
            printf("Undefined variable: %s", name);
          }
          free(name);
        }
      }
    }

    else if (strncmp(p, "in", 2) == 0) {
      p += 2;
      skip_spaces(&p);
      if (*p == ':') {
        p++;
        skip_spaces(&p);
      }

      char *prompt = parse_string(&p);
      char buffer[256];

      if (prompt) {
        printf("%s", prompt);
        free(prompt);
      }

      if (fgets(buffer, sizeof(buffer), stdin)) {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n')
          buffer[len - 1] = '\0';
      }

      // cek kalau ada @varName
      skip_spaces(&p);
      if (*p == '@') {
        p++;
        char *varName = parse_word(&p);
        if (varName) {
          struct VAR_IN *v = find_var(varName);
          if (v && strcmp(v->type, "string") == 0) {
            free(v->strval);            // hapus lama
            v->strval = strdup(buffer); // set ulang value baru
          }
          free(varName);
        }
      } else {
        // fallback: simpan ke lastInput
        struct VAR_IN *v = find_var("lastInput");
        if (!v && var_count < MAX_VARS) {
          strcpy(vars[var_count].name, "lastInput");
          strcpy(vars[var_count].type, "string");
          vars[var_count].strval = strdup(buffer);
          vars[var_count].isActive = 1;
          var_count++;
        } else if (v) {
          free(v->strval);
          v->strval = strdup(buffer);
        }
      }
    }

    else if (strncmp(p, "str", 3) == 0) {
      p += 3;
      skip_spaces(&p);
      char *identifier = parse_word(&p);
      skip_spaces(&p);
      if (*p == '=') {
        p++;
        skip_spaces(&p);
        char *val = parse_string(&p);
        if (identifier && val && var_count < MAX_VARS) {
          strcpy(vars[var_count].name, identifier);
          strcpy(vars[var_count].type, "string");
          vars[var_count].strval = strdup(val);
          vars[var_count].isActive = 1;
          var_count++;
        }
        if (val)
          free(val);
      }
      if (identifier)
        free(identifier);
    }

    else if (strncmp(p, "newline", 7) == 0) {
      printf("\n");
      p += 7;
    }

    else if (strncmp(p, "bool", 4) == 0) {
      p += 4;
      skip_spaces(&p);
      char *identifier = parse_word(&p);
      skip_spaces(&p);
      int val = 0;
      if (*p == '=') {
        p++;
        skip_spaces(&p);
        char *word = parse_word(&p);
        if (word) {
          if (strcmp(word, "true") == 0)
            val = 1;
          free(word);
        }
      }
      if (identifier && var_count < MAX_VARS) {
        strcpy(vars[var_count].name, identifier);
        strcpy(vars[var_count].type, "bool");
        vars[var_count].boolval = val;
        vars[var_count].isActive = 1;
        var_count++;
      }
      if (identifier)
        free(identifier);
    } else if (strncmp(p, "chckVarLs", 9) == 0) {
      p += 9;
      for (int i = 0; i < var_count; i++) {
        printf("Name: %s\n", vars[i].name);
        if (vars[i].isActive) {
          if (strcmp(vars[i].type, "string") == 0 && vars[i].strval) {
            printf(" Value: %s\n", vars[i].strval);
          } else if (strcmp(vars[i].type, "bool") == 0) {
            printf(" Value: %s\n", vars[i].boolval ? "true" : "false");
          } else if (strcmp(vars[i].type, "int") == 0) {
            printf(" Value: %d\n", vars[i].intval);
          }
        }
      }
    }

    else {
      printf("Unknown keyword\n");
      break;
    }

    skip_spaces(&p);
  }

  free(src);
}
