#ifndef CLIBP_CONFP_H
  #define CLIBP_CONFP_H

#include <stdio.h>
#include <stdbool.h>

static const char* clibp_confp_skip(const char* src);
static const char* clibp_confp_section(const char* src, char* name);
static const char* clibp_confp_identifier(const char* src, char* dump);
static const char* clibp_confp_string(const char* src, char* dump);
static const char* clibp_confp_integer(const char* src, int* dump);
static const char* clibp_confp_boolean(const char* src, bool* dump);

static const char* clibp_confp_skip(const char* src)
{
  while (1)
  {
    if (*src == '#')
    {
      while (*src != '\0' && *src++ != '\n');
      continue;
    }else if (*src != ' ' && *src != '\t' && *src != '\n')
      break;

    src++;
  }
  return src;
}

static const char* clibp_confp_section(const char* src, char* name)
{
  src = clibp_confp_skip(src);

  if (*src != '[')
  {
    fprintf(stderr, "clibp_confp_section(): expected '['\n");
    return NULL;
  }

  while (1)
  {
    if ((*src >= 'A' && *src <= 'Z') || (*src >= 'a' && *src <= 'z') || *src == '-' || *src == '_')
    {
      *name++ = *src++;
      continue;
    }
    break;
  }

  if (*src != ']')
  {
    fprintf(stderr, "clibp_confp_section(): expected ']'\n");
    return NULL;
  }

  *name = '\0';
  return src;
}

static const char* clibp_confp_identifier(const char* src, char* dump)
{
  while (1)
  {
    if ((*src >= 'A' && *src <= 'Z') || (*src >= 'a' && *src <= 'z') || *src == '-' || *src == '_')
    {
      *dump++ = *src++;
      continue;
    }
    break;
  }

  if (*src != '=')
  {
    fprintf(stderr, "clibp_confp_identifier(): expected '='\n");
    return NULL;
  }

  *dump = '\0';
  return src;
}

static const char* clibp_confp_string(const char* src, char* dump)
{
  if (*src != '"')
  {
    fprintf(stderr, "clibp_conf_string(): expected '\"'\n");
    return NULL;
  }

  src++;

  while (1)
  {
    if (*src == '\0')
    {
      fprintf(stderr, "clibp_conf_string(): expected end '\"'\n");
      return NULL;
    }

    if (*src == '\"')
      break;

    *dump = *src++;
  }

  *dump = '\0';
  src++;
  return src;
}

static const char* clibp_confp_integer(const char* src, int* dump)
{
  return src;
}

static const char* clibp_confp_boolean(const char* src, bool* dump)
{
  return src;
}

#endif // CLIB_CONFP_H
