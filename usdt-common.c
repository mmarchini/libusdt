#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#include "usdt.h"

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

usdt_provider_t *
usdt_create_provider(const char *name, const char *module)
{
        usdt_provider_t *provider;

        if ((provider = malloc(sizeof *provider)) == NULL)
                return NULL;

        provider->name = strdup(name);
        provider->module = strdup(module);
        provider->probedefs = NULL;
        provider->enabled = 0;

        return provider;
}

usdt_probedef_t *
usdt_create_probe(const char *func, const char *name, size_t argc, const char **types)
{
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

int
usdt_provider_add_probe(usdt_provider_t *provider, usdt_probedef_t *probedef)
{
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

int
usdt_provider_remove_probe(usdt_provider_t *provider, usdt_probedef_t *probedef)
{
        usdt_probedef_t *pd, *prev_pd = NULL;

        if (provider->probedefs == NULL) {
                usdt_error(provider, USDT_ERROR_NOPROBES);
                return (-1);
        }

        for (pd = provider->probedefs; (pd != NULL);
             prev_pd = pd, pd = pd->next) {

                if ((strcmp(pd->name, probedef->name) == 0) &&
                    (strcmp(pd->function, probedef->function) == 0)) {

                        if (prev_pd == NULL)
                                provider->probedefs = pd->next;
                        else
                                prev_pd->next = pd->next;

                        return (0);
                }
        }

        usdt_error(provider, USDT_ERROR_REMOVE_PROBE,
                   provider->name, provider->module,
                   probedef->function, probedef->name);
        return (-1);
}

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
