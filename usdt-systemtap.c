#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <libstapsdt.h>

#include "usdt.h"

ArgType_t stringToArgType(char *type) {
  // TODO (mmarchini) better mapping of arguments, but for now this works
  return uint64;
}

void usdt_fire_probe(usdt_probe_t *probe, size_t argc, void **argv) {
  if (probe == NULL)
    return;

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
        stapProbe = providerAddProbe(stapProvider, node->name, node->argc,
          stringToArgType(((node->types))[0]));
        break;
      case 2:
        stapProbe = providerAddProbe(stapProvider, node->name, node->argc,
          stringToArgType(((node->types))[0]),
          stringToArgType(((node->types))[1]));
        break;
      case 3:
        stapProbe = providerAddProbe(stapProvider, node->name, node->argc,
          stringToArgType(((node->types))[0]),
          stringToArgType(((node->types))[1]),
          stringToArgType(((node->types))[2]));
        break;
      case 4:
        stapProbe = providerAddProbe(stapProvider, node->name, node->argc,
          stringToArgType(((node->types))[0]),
          stringToArgType(((node->types))[1]),
          stringToArgType(((node->types))[2]),
          stringToArgType(((node->types))[3]));
        break;
      case 5:
        stapProbe = providerAddProbe(stapProvider, node->name, node->argc,
          stringToArgType(((node->types))[0]),
          stringToArgType(((node->types))[1]),
          stringToArgType(((node->types))[2]),
          stringToArgType(((node->types))[3]),
          stringToArgType(((node->types))[4]));
        break;
      case 6:
        stapProbe = providerAddProbe(stapProvider, node->name, node->argc,
          stringToArgType(((node->types))[0]),
          stringToArgType(((node->types))[1]),
          stringToArgType(((node->types))[2]),
          stringToArgType(((node->types))[3]),
          stringToArgType(((node->types))[4]),
          stringToArgType(((node->types))[5]));
        break;
      default:
        stapProbe = providerAddProbe(stapProvider, node->name, 0);
    }
    node->probe->probe_addr = (void *)stapProbe;
  }
  provider->file = (void *)stapProvider;
  if(providerLoad(stapProvider) == 0) {
    provider->enabled = 1;
    return 0;
  }
  else {
    return -1;
  }
}

int usdt_is_enabled(usdt_probe_t *probe) {
  if(probe == NULL) {
    return 0;
  }
  if(probe->probe_addr == NULL) {
    return 0;
  }
  return probeIsEnabled((SDTProbe_t *) probe->probe_addr);
}

// XXX skip implementation for now

void free_probedef(usdt_probedef_t *pd) {
  int i;

  free((char *)pd->function);
  free((char *)pd->name);

  pd->probe = NULL;

  for (i = 0; i < pd->argc; i++) {
    free(pd->types[i]);
  }

  free(pd);
}

void usdt_probe_release(usdt_probedef_t *probedef) {
  free_probedef(probedef);
}

int usdt_provider_disable(usdt_provider_t *provider) {
  usdt_probedef_t *pd;

  if (provider->enabled == 0)
          return (0);

  if ((providerUnload((SDTProvider_t *)provider->file)) < 0) {
          usdt_error(provider, USDT_ERROR_UNLOADDOF, strerror(errno));
          return (-1);
  }

  providerDestroy((SDTProvider_t *)provider->file);
  provider->file = NULL;

  /* providerDestroy already frees all probes, so we only need to set them to NULL here */
  for (pd = provider->probedefs; (pd != NULL); pd = pd->next) {
    if (pd->probe) {
      pd->probe = NULL;
    }
  }

  provider->enabled = 0;

  return (0);
}

void usdt_provider_free(usdt_provider_t *provider) {
  usdt_probedef_t *pd, *next;

  for (pd = provider->probedefs; pd != NULL; pd = next) {
    next = pd->next;
    free_probedef(pd);
  }

  free((char *)provider->name);
  free((char *)provider->module);
  free(provider);
}
