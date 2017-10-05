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
  SDTProbe_t *sdtProbe;
  if(probe->probe_addr == NULL) {
    return 0;
  }
  return probeIsEnabled((SDTProbe_t *) probe->probe_addr);
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
