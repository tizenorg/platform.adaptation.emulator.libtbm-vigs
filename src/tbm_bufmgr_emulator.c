#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <tbm_bufmgr.h>
#include <tbm_bufmgr_backend.h>
#include "vigs.h"
#include "tbm_emulator_log.h"
#include <string.h>
#include <stdlib.h>

struct tbm_bo_emulator
{
    struct vigs_drm_surface *sfc;

    int is_write;
};

static tbm_bo_handle get_tbm_bo_handle(struct vigs_drm_gem *gem,
                                       int device)
{
    tbm_bo_handle bo_handle;
    int ret;

    memset(&bo_handle, 0, sizeof(bo_handle));

    switch (device) {
    case TBM_DEVICE_DEFAULT:
    case TBM_DEVICE_2D:
        bo_handle.u32 = gem->handle;
        break;
    case TBM_DEVICE_CPU:
        ret = vigs_drm_gem_map(gem);

        if (ret == 0) {
            bo_handle.ptr = gem->vaddr;
        } else {
            TBM_EMULATOR_LOG_ERROR("vigs_drm_gem_map failed: %s",
                                   strerror(-ret));
        }

        break;
    case TBM_DEVICE_3D:
        TBM_EMULATOR_LOG_ERROR("TBM_DEVICE_3D not supported");
        break;
    case TBM_DEVICE_MM:
        TBM_EMULATOR_LOG_ERROR("TBM_DEVICE_MM not supported");
        break;
    default:
        TBM_EMULATOR_LOG_ERROR("%d not supported", device);
        break;
    }

    return bo_handle;
}

static void tbm_bufmgr_emulator_deinit(void *priv)
{
    struct vigs_drm_device *drm_dev = priv;

    TBM_EMULATOR_LOG_DEBUG("enter");

    vigs_drm_device_destroy(drm_dev);
}

static int tbm_bufmgr_emulator_bo_size(tbm_bo bo)
{
    struct tbm_bo_emulator *bo_emulator;

    TBM_EMULATOR_LOG_DEBUG("bo = %p", bo);

    bo_emulator = (struct tbm_bo_emulator*)tbm_backend_get_bo_priv(bo);

    return bo_emulator->sfc->gem.size;
}

static void *tbm_bufmgr_emulator_bo_alloc(tbm_bo bo, int size, int flags)
{
    TBM_EMULATOR_LOG_ERROR("not supported");
    return NULL;
}

static void tbm_bufmgr_emulator_bo_free(tbm_bo bo)
{
    struct tbm_bo_emulator *bo_emulator;

    TBM_EMULATOR_LOG_DEBUG("bo = %p", bo);

    bo_emulator = (struct tbm_bo_emulator*)tbm_backend_get_bo_priv(bo);

    vigs_drm_gem_unref(&bo_emulator->sfc->gem);

    free(bo_emulator);
}

static void *tbm_bufmgr_emulator_bo_import(tbm_bo bo, unsigned int key)
{
    struct vigs_drm_device *drm_dev;
    int ret;
    struct vigs_drm_surface *sfc;
    struct tbm_bo_emulator *bo_emulator;

    TBM_EMULATOR_LOG_DEBUG("bo = %p, key = %u", bo, key);

    drm_dev = (struct vigs_drm_device*)tbm_backend_get_bufmgr_priv(bo);

    ret = vigs_drm_surface_open(drm_dev, key, &sfc);

    if (ret != 0) {
        TBM_EMULATOR_LOG_ERROR("vigs_drm_surface_open failed for key %u: %s",
                               key,
                               strerror(-ret));
        return NULL;
    }

    bo_emulator = calloc(1, sizeof(*bo_emulator));

    if (!bo_emulator) {
        TBM_EMULATOR_LOG_ERROR("calloc failed");
        vigs_drm_gem_unref(&sfc->gem);
        return NULL;
    }

    bo_emulator->sfc = sfc;

    TBM_EMULATOR_LOG_DEBUG("handle = %u", sfc->gem.handle);

    return bo_emulator;
}

static unsigned int tbm_bufmgr_emulator_bo_export(tbm_bo bo)
{
    struct tbm_bo_emulator *bo_emulator;
    int ret;

    TBM_EMULATOR_LOG_DEBUG("bo = %p", bo);

    bo_emulator = (struct tbm_bo_emulator*)tbm_backend_get_bo_priv(bo);

    ret = vigs_drm_gem_get_name(&bo_emulator->sfc->gem);

    if (ret != 0) {
        TBM_EMULATOR_LOG_ERROR("vigs_drm_gem_get_name failed: %s",
                               strerror(-ret));
        return 0;
    }

    return bo_emulator->sfc->gem.name;
}

static tbm_bo_handle tbm_bufmgr_emulator_bo_get_handle(tbm_bo bo, int device)
{
    struct tbm_bo_emulator *bo_emulator;

    TBM_EMULATOR_LOG_DEBUG("bo = %p, device = %d", bo, device);

    bo_emulator = (struct tbm_bo_emulator*)tbm_backend_get_bo_priv(bo);

    return get_tbm_bo_handle(&bo_emulator->sfc->gem, device);
}

static tbm_bo_handle tbm_bufmgr_emulator_bo_map(tbm_bo bo, int device, int opt)
{
    struct tbm_bo_emulator *bo_emulator;
    tbm_bo_handle handle;

    TBM_EMULATOR_LOG_DEBUG("bo = %p, device = %d, opt = %d", bo, device, opt);

    bo_emulator = (struct tbm_bo_emulator*)tbm_backend_get_bo_priv(bo);

    handle = get_tbm_bo_handle(&bo_emulator->sfc->gem, device);

    if (!handle.ptr) {
        return handle;
    }

    if ((opt & TBM_OPTION_READ) != 0) {
        vigs_drm_surface_update_vram(bo_emulator->sfc);
    }

    if ((opt & TBM_OPTION_WRITE) != 0) {
        bo_emulator->is_write = 1;
    }

    return handle;
}

static int tbm_bufmgr_emulator_bo_unmap(tbm_bo bo)
{
    struct tbm_bo_emulator *bo_emulator;

    TBM_EMULATOR_LOG_DEBUG("bo = %p", bo);

    bo_emulator = (struct tbm_bo_emulator*)tbm_backend_get_bo_priv(bo);

    if (bo_emulator->is_write) {
        vigs_drm_surface_update_gpu(bo_emulator->sfc);
        bo_emulator->is_write = 0;
    }

    return 1;
}

static int tbm_bufmgr_emulator_bo_cache_flush(tbm_bo bo, int flags)
{
    TBM_EMULATOR_LOG_DEBUG("bo = %p, flags = %d", bo, flags);
    return 1;
}

static int tbm_bufmgr_emulator_bo_get_global_key(tbm_bo bo)
{
    struct tbm_bo_emulator *bo_emulator;
    int ret;

    TBM_EMULATOR_LOG_DEBUG("bo = %p", bo);

    bo_emulator = (struct tbm_bo_emulator*)tbm_backend_get_bo_priv(bo);

    ret = vigs_drm_gem_get_name(&bo_emulator->sfc->gem);

    if (ret != 0) {
        TBM_EMULATOR_LOG_ERROR("vigs_drm_gem_get_name failed: %s",
                               strerror(-ret));
        return 0;
    }

    return bo_emulator->sfc->gem.name;
}

MODULEINITPPROTO(tbm_bufmgr_emulator_init);

static TBMModuleVersionInfo EmulatorVersRec =
{
    "emulator",
    "Samsung",
    TBM_ABI_VERSION,
};

TBMModuleData tbmModuleData = { &EmulatorVersRec, tbm_bufmgr_emulator_init };

int tbm_bufmgr_emulator_init(tbm_bufmgr bufmgr, int fd)
{
    int ret = 0;
    struct vigs_drm_device *drm_dev = NULL;
    tbm_bufmgr_backend backend = NULL;

    TBM_EMULATOR_LOG_DEBUG("enter");

    if (!bufmgr) {
        return 0;
    }

    ret = vigs_drm_device_create(fd, &drm_dev);

    if (ret != 0) {
        TBM_EMULATOR_LOG_ERROR("vigs_drm_device_create failed: %s", strerror(-ret));
        goto fail;
    }

    backend = tbm_backend_alloc();

    if (!backend) {
        TBM_EMULATOR_LOG_ERROR("tbm_backend_alloc failed");
        goto fail;
    }

    backend->flags = 0;
    backend->priv = (void*)drm_dev;
    backend->bufmgr_deinit = tbm_bufmgr_emulator_deinit;
    backend->bo_size = tbm_bufmgr_emulator_bo_size;
    backend->bo_alloc = tbm_bufmgr_emulator_bo_alloc;
    backend->bo_free = tbm_bufmgr_emulator_bo_free;
    backend->bo_import = tbm_bufmgr_emulator_bo_import;
    backend->bo_export = tbm_bufmgr_emulator_bo_export;
    backend->bo_get_handle = tbm_bufmgr_emulator_bo_get_handle;
    backend->bo_map = tbm_bufmgr_emulator_bo_map;
    backend->bo_unmap = tbm_bufmgr_emulator_bo_unmap;
    backend->bo_cache_flush = tbm_bufmgr_emulator_bo_cache_flush;
    backend->bo_get_global_key = tbm_bufmgr_emulator_bo_get_global_key;
    backend->bo_lock = NULL;
    backend->bo_unlock = NULL;

    if (!tbm_backend_init(bufmgr, backend)) {
        TBM_EMULATOR_LOG_ERROR("tbm_backend_init failed");
        goto fail;
    }

    TBM_EMULATOR_LOG_INFO("initialized");

    return 1;

fail:
    if (backend) {
        tbm_backend_free(backend);
    }

    if (drm_dev) {
        vigs_drm_device_destroy(drm_dev);
    }

    return 0;
}
