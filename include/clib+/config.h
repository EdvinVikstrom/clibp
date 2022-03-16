#ifndef CLIBP_CONFP_H
  #define CLIBP_CONFP_H

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

enum config_variable_types {
  CONFIG_VARIABLE_TYPE_STRING,
  CONFIG_VARIABLE_TYPE_ARRAY
};

struct config_variable {
  const char* name;

  enum config_variable_types type;
  union {
    struct {
      char* begin;
      size_t len;
    } string;
    struct {
      struct config_variable* begin;
      size_t len;
    } array;
  } value;
};

struct config_section {
  const char* name;
  size_t var_count;
  struct config_variable* vars;
};

struct config {
  size_t sec_count;
  struct config_section* secs;
};

static int config_parser_next(const char** data);
static int config_parser_identifier(const char** data, char* dump);
static int config_parser_string(const char** data, size_t* len, char* str);
static int config_parser_array(const char** data, size_t* len, struct config_variable* vars);
static int config_parser_variable(const char** data, struct config_variable* variable);
static int config_parser_section(const char** data, struct config_section* section);
static int config_parser_load(const char* data, struct config* conf);

static int config_parser_next(const char** data)
{
  const char* iter = *data;

  while (1)
  {
    if (*iter == ' ' || *iter == '\t' || *iter == '\n')
    {
      iter++;
      continue;
    }

    if (*iter == '#')
    {
      while (*iter != '\0' && *iter++ != '\n'); // may not work
      continue;
    }

    break;
  }

  if ((*iter >= 'A' && *iter <= 'Z') || (*iter >= 'a' && *iter <= 'z'))
  {
    *data = iter;
    return 1;
  }

  switch (*iter)
  {
    case '-':
    case '_':
      *data = iter;
      return 1;

    case '[':
      *data = iter + 1;
      return 2;

    case '\"':
    case '\'':
      *data = iter + 1;
      return 3;

    case '(':
      *data = iter + 1;
      return 4;

    case '\0':
      *data = iter;
      return -1;

    default:
      *data = iter;
      return 0;
  }
}

static int config_parser_identifier(const char** data, char* dump)
{
  const char* iter = *data;

  while (1)
  {
    if ((*iter >= 'A' && *iter <= 'Z') || (*iter >= 'a' && *iter <= 'z') || *iter == '-' || *iter == '_')
    {
      *dump++ = *iter++;
      continue;
    }

    break;
  }

  *dump = '\0';
  *data = iter;
  return 0;
}

static int config_parser_string(const char** data, size_t* len, char* str)
{
  const char* iter = *data;

  if (str == NULL)
  {
    size_t count = 0;

    while (1)
    {
      if (*iter == '\0')
      {
	fprintf(stderr, "config_parser_string(): expected ending '\"'\n");
	return 1;
      }else if (*iter == '\"' || *iter == '\'')
      {
	iter++;
	break;
      }

      iter++;
      count++;
    }

    *data = iter;
    *len = count;
    return 0;
  }

  memcpy(str, iter, *len);
  str[*len] = '\0';

  *data = iter + *len + 1;
  return 0;
}

static int config_parser_array(const char** data, size_t* len, struct config_variable* vars)
{
  const char* iter = *data;

  if (vars == NULL)
  {
    size_t count = 0;

    while (1)
    {
      if (*iter == '\0')
      {
	fprintf(stderr, "config_parser_array(): expected ending ')'\n");
	return 1;
      }else if (*iter == ')')
      {
	iter++;
	break;
      }

      if (config_parser_variable(&iter, NULL))
	return 1;

      count++;
    }

    *data = iter;
    *len = count;
    return 0;
  }

  for (size_t i = 0; i != *len; i++)
  {
    config_parser_variable(&iter, &vars[i]);
  }

  *data = iter + 1;
  return 0;
}

static int config_parser_variable(const char** data, struct config_variable* variable)
{
  const char* iter = *data;

  int type = config_parser_next(&iter);

  if (type == 3)
  {
    const char* tmp = iter;
    size_t len = 0;

    if (config_parser_string(&tmp, &len, NULL))
      return 1;

    if (variable != NULL)
    {
      variable->value.string.begin = (char*) malloc(len + 1);
      variable->value.string.len = len;
      variable->type = CONFIG_VARIABLE_TYPE_STRING;

      config_parser_string(&iter, &len, variable->value.string.begin);
    }else
      iter = tmp;
  }else if (type == 4)
  {
    const char* tmp = iter;
    size_t len = 0;

    if (config_parser_array(&tmp, &len, NULL))
      return 1;

    if (variable != NULL)
    {
      variable->value.array.begin = (struct config_variable*) malloc(sizeof(struct config_variable) * len);
      variable->value.array.len = len;
      variable->type = CONFIG_VARIABLE_TYPE_ARRAY;

      config_parser_array(&iter, &len, variable->value.array.begin);
    }else
      iter = tmp;
  }

  *data = iter;
  return 0;
}

static int config_parser_section(const char** data, struct config_section* section)
{
  const char* iter = *data;

  int type;
  char name[64];

  while (1)
  {
    type = config_parser_next(&iter);

    if (type == 1)
    {
      config_parser_identifier(&iter, name);

      if (*iter != '=')
      {
	fprintf(stderr, "config_parser_section(): expected '=' after identifier\n");
	return 1;
      }
      iter++;

      struct config_variable* variable = NULL;

      for (size_t i = 0; i < section->var_count; i++)
      {
	if (strcmp(name, section->vars[i].name) == 0)
	{
	  variable = &section->vars[i];
	  break;
	}
      }

      if (variable == NULL)
      {
	fprintf(stderr, "config_parser_section(): unknown variable '%s'\n", name);
	return 1;
      }

      if (config_parser_variable(&iter, variable))
	return 0;

      continue;
    }else if (type == 2)
    {
      iter--;
      break;
    }

    break;
  }

  *data = iter;
  return 0;
}

static int config_parser_load(const char* data, struct config* conf)
{
  const char* iter = data;

  int type;
  char name[64];

  while (1)
  {
    type = config_parser_next(&iter);

    if (type == 2)
    {
      config_parser_identifier(&iter, name);

      if (*iter != ']')
      {
	fprintf(stderr, "config_parser_load(): expected ending ']'\n");
	return 1;
      }
      iter++;

      struct config_section* section = NULL;

      for (size_t i = 0; i < conf->sec_count; i++)
      {
	if (strcmp(name, conf->secs[i].name) == 0)
	{
	  section = &conf->secs[i];
	  break;
	}
      }

      if (section == NULL)
      {
	fprintf(stderr, "config_parser_load(): unknown section [%s]\n", name);
	return 1;
      }

      if (config_parser_section(&iter, section))
	return 1;
    }else if (type == -1)
      break;
  }
  return 0;
}

#endif // CLIB_CONFP_H
