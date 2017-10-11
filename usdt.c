/*
 * Copyright (c) 2012, Chris Andrews. All rights reserved.
 */

#include "usdt_internal.h"

#include <stdarg.h>
#include <string.h>
#include <stdio.h>

static void
free_probedef(usdt_probedef_t *pd)
{
        int i;

        switch (pd->refcnt) {
        case 1:
                free((char *)pd->function);
                free((char *)pd->name);
                if (pd->probe) {
                        usdt_free_tracepoints(pd->probe);
                        free(pd->probe);
                }
                for (i = 0; i < pd->argc; i++)
                        free(pd->types[i]);
                free(pd);
                break;
        case 2:
                pd->refcnt = 1;
                break;
        default:
                break;
        }
}

void
usdt_probe_release(usdt_probedef_t *probedef)
{
        free_probedef(probedef);
}

int
usdt_provider_enable(usdt_provider_t *provider)
{
        usdt_strtab_t strtab;
        usdt_dof_file_t *file;
        usdt_probedef_t *pd;
        int i;
        size_t size;
        usdt_dof_section_t sects[5];

        if (provider->enabled == 1) {
                usdt_error(provider, USDT_ERROR_ALREADYENABLED);
                return (0); /* not fatal */
        }

        if (provider->probedefs == NULL) {
                usdt_error(provider, USDT_ERROR_NOPROBES);
                return (-1);
        }

        for (pd = provider->probedefs; pd != NULL; pd = pd->next) {
                if ((pd->probe = malloc(sizeof(*pd->probe))) == NULL) {
                        usdt_error(provider, USDT_ERROR_MALLOC);
                        return (-1);
                }
        }

        if ((usdt_strtab_init(&strtab, 0)) < 0) {
                usdt_error(provider, USDT_ERROR_MALLOC);
                return (-1);
        }

        if ((usdt_strtab_add(&strtab, provider->name)) == 0) {
                usdt_error(provider, USDT_ERROR_MALLOC);
                return (-1);
        }

        if ((usdt_dof_probes_sect(&sects[0], provider, &strtab)) < 0)
                return (-1);
        if ((usdt_dof_prargs_sect(&sects[1], provider)) < 0)
                return (-1);

        size = usdt_provider_dof_size(provider, &strtab);
        if ((file = usdt_dof_file_init(provider, size)) == NULL)
                return (-1);

        if ((usdt_dof_proffs_sect(&sects[2], provider, file->dof)) < 0)
                return (-1);
        if ((usdt_dof_prenoffs_sect(&sects[3], provider, file->dof)) < 0)
                return (-1);
        if ((usdt_dof_provider_sect(&sects[4], provider)) < 0)
                return (-1);

        for (i = 0; i < 5; i++)
                usdt_dof_file_append_section(file, &sects[i]);

        usdt_dof_file_generate(file, &strtab);

        usdt_dof_section_free((usdt_dof_section_t *)&strtab);
        for (i = 0; i < 5; i++)
                usdt_dof_section_free(&sects[i]);

        if ((usdt_dof_file_load(file, provider->module)) < 0) {
                usdt_error(provider, USDT_ERROR_LOADDOF, strerror(errno));
                return (-1);
        }

        provider->enabled = 1;
        provider->file = file;

        return (0);
}

int
usdt_provider_disable(usdt_provider_t *provider)
{
        usdt_probedef_t *pd;

        if (provider->enabled == 0)
                return (0);

        if ((usdt_dof_file_unload((usdt_dof_file_t *)provider->file)) < 0) {
                usdt_error(provider, USDT_ERROR_UNLOADDOF, strerror(errno));
                return (-1);
        }

        usdt_dof_file_free(provider->file);
        provider->file = NULL;

        /* We would like to free the tracepoints here too, but OS X
         * (and to a lesser extent Illumos) struggle with this:
         *
         * If a provider is repeatedly disabled and re-enabled, and is
         * allowed to reuse the same memory for its tracepoints, *and*
         * there's a DTrace consumer running with enablings for these
         * probes, tracepoints are not always cleaned up sufficiently
         * that the newly-created probes work.
         *
         * Here, then, we will leak the memory holding the
         * tracepoints, which serves to stop us reusing the same
         * memory address for new tracepoints, avoiding the bug.
         */

        for (pd = provider->probedefs; (pd != NULL); pd = pd->next) {
                /* may have an as yet never-enabled probe on an
                   otherwise enabled provider */
                if (pd->probe) {
                        /* usdt_free_tracepoints(pd->probe); */
                        free(pd->probe);
                        pd->probe = NULL;
                }
        }

        provider->enabled = 0;

        return (0);
}

void
usdt_provider_free(usdt_provider_t *provider)
{
        usdt_probedef_t *pd, *next;

        for (pd = provider->probedefs; pd != NULL; pd = next) {
                next = pd->next;
                free_probedef(pd);
        }

        free((char *)provider->name);
        free((char *)provider->module);
        free(provider);
}

int
usdt_is_enabled(usdt_probe_t *probe)
{
        if (probe != NULL)
                return (*probe->isenabled_addr)();
        else
                return 0;
}

void
usdt_fire_probe(usdt_probe_t *probe, size_t argc, void **nargv)
{
        if (probe != NULL)
                usdt_probe_args(probe->probe_addr, argc, nargv);
}
