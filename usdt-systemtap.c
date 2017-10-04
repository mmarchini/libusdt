#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <libstapsdt.h>

#include "usdt.h"


void usdt_fire_probe(usdt_probe_t *probe, size_t argc, void **argv) {
  // XXX IMPLEMENT NOW
  switch(argc) {
    case 0:
      probeFire((SDTProbe_t *)probe->probe_addr);
      break;
    case 1:
      probeFire((SDTProbe_t *)probe->probe_addr, argv[0]);
      break;
    case 2:
      probeFire((SDTProbe_t *)probe->probe_addr, argv[0], argv[1]);
      break;
    case 3:
      probeFire((SDTProbe_t *)probe->probe_addr, argv[0], argv[1], argv[2]);
      break;
    case 4:
      probeFire((SDTProbe_t *)probe->probe_addr, argv[0], argv[1], argv[2], argv[3]);
      break;
    case 5:
      probeFire((SDTProbe_t *)probe->probe_addr, argv[0], argv[1], argv[2], argv[3], argv[4]);
      break;
    case 6:
      probeFire((SDTProbe_t *)probe->probe_addr, argv[0], argv[1], argv[2], argv[3], argv[4], argv[5]);
      break;
    default:
      probeFire((SDTProbe_t *)probe->probe_addr);
  }
}

int usdt_provider_enable(usdt_provider_t *provider) {
  // XXX IMPLEMENT NOW
  SDTProvider_t *stapProvider = providerInit(provider->name);
  SDTProbe_t *stapProbe;
  for(usdt_probedef_t *node=provider->probedefs; node!=NULL; node=node->next) {
    node->probe = calloc(sizeof(usdt_probe_t), 1);
    node->probe->isenabled_addr = NULL;
    // FIXME refactor
    switch(node->argc) {
      case 0:
        stapProbe = providerAddProbe(stapProvider, node->name, node->argc);
        break;
      case 1:
        stapProbe = providerAddProbe(stapProvider, node->name, node->argc, (*(node->types))[0]);
        break;
      case 2:
        stapProbe = providerAddProbe(stapProvider, node->name, node->argc, (*(node->types))[0], (*(node->types))[1]);
        break;
      case 3:
        stapProbe = providerAddProbe(stapProvider, node->name, node->argc, (*(node->types))[0], (*(node->types))[1], (*(node->types))[2]);
        break;
      case 4:
        stapProbe = providerAddProbe(stapProvider, node->name, node->argc, (*(node->types))[0], (*(node->types))[1], (*(node->types))[2], (*(node->types))[3]);
        break;
      case 5:
        stapProbe = providerAddProbe(stapProvider, node->name, node->argc, (*(node->types))[0], (*(node->types))[1], (*(node->types))[2], (*(node->types))[3], (*(node->types))[4]);
        break;
      case 6:
        stapProbe = providerAddProbe(stapProvider, node->name, node->argc, (*(node->types))[0], (*(node->types))[1], (*(node->types))[2], (*(node->types))[3], (*(node->types))[4], (*(node->types))[5]);
        break;
      default:
        stapProbe = providerAddProbe(stapProvider, node->name, 0);
    }
    node->probe->probe_addr = (void *)stapProbe;
  }
  provider->file = (void *)stapProvider;
  return providerLoad(stapProvider);
}

// XXX skip implementation for now

void usdt_probe_release(usdt_probedef_t *probedef) {
  printf("usdt_probe_release: not implemented yet!\n");
}

int usdt_is_enabled(usdt_probe_t *probe) {
  printf("usdt_is_enabled: not implemented yet!\n");
  return 1;
}

int usdt_provider_remove_probe(usdt_provider_t *provider, usdt_probedef_t *probedef) {
  printf("usdt_provider_remove_probe: not implemented yet!");
  return 0;
}

int usdt_provider_disable(usdt_provider_t *provider) {
  printf("usdt_provider_disable: not implemented yet!");
  return 0;
}
void usdt_provider_free(usdt_provider_t *provider) {
  printf("usdt_provider_free: not implemented yet!");
}

// XXX Same imeplentations
char *usdt_errors[] = {
  "failed to allocate memory",
  "failed to allocate page-aligned memory",
  "no probes defined",
  "failed to load DOF: %s",
  "provider is already enabled",
  "failed to unload DOF: %s",
  "probe named %s:%s:%s:%s already exists",
  "failed to remove probe %s:%s:%s:%s"
};

static void
usdt_verror(usdt_provider_t *provider, usdt_error_t error, va_list argp)
{
        (void)vasprintf(&provider->error, usdt_errors[error], argp);
}

void usdt_error(usdt_provider_t *provider, usdt_error_t error, ...) {
  va_list argp;

  va_start(argp, error);
  usdt_verror(provider, error, argp);
  va_end(argp);
}
char *usdt_errstr(usdt_provider_t *provider) {
  return (provider->error);
}

usdt_probedef_t *usdt_create_probe(const char *func, const char *name,
                                   size_t argc, const char **types) {
  int i;
  usdt_probedef_t *p;

  if (argc > USDT_ARG_MAX)
          argc = USDT_ARG_MAX;

  if ((p = malloc(sizeof *p)) == NULL)
          return (NULL);

  p->refcnt = 2;
  p->function = strdup(func);
  p->name = strdup(name);
  p->argc = argc;
  p->probe = NULL;

  for (i = 0; i < argc; i++)
          p->types[i] = strdup(types[i]);

  return (p);
}

usdt_provider_t *usdt_create_provider(const char *name, const char *module) {
  usdt_provider_t *provider;

  if ((provider = malloc(sizeof *provider)) == NULL)
          return NULL;

  provider->name = strdup(name);
  provider->module = strdup(module);
  provider->probedefs = NULL;
  provider->enabled = 0;

  return provider;
}

int usdt_provider_add_probe(usdt_provider_t *provider, usdt_probedef_t *probedef) {
  usdt_probedef_t *pd;
  if (provider->probedefs != NULL) {
          for (pd = provider->probedefs; (pd != NULL); pd = pd->next) {
          if ((strcmp(pd->name, probedef->name) == 0) &&
              (strcmp(pd->function, probedef->function) == 0)) {
                          usdt_error(provider, USDT_ERROR_DUP_PROBE,
                                     provider->name, provider->module,
                                     probedef->function, probedef->name);
                          return (-1);
                  }
          }
  }

  probedef->next = NULL;
  if (provider->probedefs == NULL)
          provider->probedefs = probedef;
  else {
          for (pd = provider->probedefs; (pd->next != NULL); pd = pd->next) ;
          pd->next = probedef;
  }

  return (0);
}
