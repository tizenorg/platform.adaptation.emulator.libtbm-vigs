#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <tbm_bufmgr.h>
#include <tbm_bufmgr_backend.h>
#include "vigs.h"
#include "tbm_emulator_log.h"
#include <string.h>

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
    TBM_EMULATOR_LOG_DEBUG("enter");
}

static int tbm_bufmgr_emulator_bo_size(tbm_bo bo)
{
    TBM_EMULATOR_LOG_DEBUG("bo = %p", bo);
    return 0;
}

static void *tbm_bufmgr_emulator_bo_alloc(tbm_bo bo, int size, int flags)
{
    TBM_EMULATOR_LOG_DEBUG("enter");
    return NULL;
}

static void tbm_bufmgr_emulator_bo_free(tbm_bo bo)
{
    TBM_EMULATOR_LOG_DEBUG("bo = %p", bo);
}

static void *tbm_bufmgr_emulator_bo_import(tbm_bo bo, unsigned int key)
{
    struct vigs_drm_device *drm_dev;
    int ret;
    struct vigs_drm_surface *sfc;

    TBM_EMULATOR_LOG_DEBUG("bo = %p, key = %u", bo, key);

    drm_dev = (struct vigs_drm_device*)tbm_backend_get_bufmgr_priv(bo);

    ret = vigs_drm_surface_open(drm_dev, key, &sfc);

    if (ret != 0) {
        TBM_EMULATOR_LOG_ERROR("vigs_drm_surface_open failed for key %u: %s",
                               key,
                               strerror(-ret));
        return NULL;
    }

    TBM_EMULATOR_LOG_DEBUG("handle = %u", sfc->gem.handle);

    return sfc;
}

static unsigned int tbm_bufmgr_emulator_bo_export(tbm_bo bo)
{
    TBM_EMULATOR_LOG_DEBUG("bo = %p", bo);
    return 0;
}

static tbm_bo_handle tbm_bufmgr_emulator_bo_get_handle(tbm_bo bo, int device)
{
    struct vigs_drm_gem *gem;

    TBM_EMULATOR_LOG_DEBUG("bo = %p, device = %d", bo, device);

    gem = (struct vigs_drm_gem*)tbm_backend_get_bo_priv(bo);

    return get_tbm_bo_handle(gem, device);
}

static tbm_bo_handle tbm_bufmgr_emulator_bo_map(tbm_bo bo, int device, int opt)
{
    struct vigs_drm_gem *gem;

    TBM_EMULATOR_LOG_DEBUG("bo = %p, device = %d, opt = %d", bo, device, opt);

    gem = (struct vigs_drm_gem*)tbm_backend_get_bo_priv(bo);

    return get_tbm_bo_handle(gem, device);
}

static int tbm_bufmgr_emulator_bo_unmap(tbm_bo bo)
{
    TBM_EMULATOR_LOG_DEBUG("bo = %p", bo);

    return 1;
}

static int tbm_bufmgr_emulator_bo_cache_flush(tbm_bo bo, int flags)
{
    TBM_EMULATOR_LOG_DEBUG("bo = %p, flags = %d", bo, flags);
    return 0;
}

static int tbm_bufmgr_emulator_bo_get_global_key(tbm_bo bo)
{
    struct vigs_drm_gem *gem;
    int ret;

    TBM_EMULATOR_LOG_DEBUG("bo = %p", bo);

    gem = (struct vigs_drm_gem*)tbm_backend_get_bo_priv(bo);

    ret = vigs_drm_gem_get_name(gem);

    if (ret != 0) {
        TBM_EMULATOR_LOG_ERROR("vigs_drm_gem_get_name failed: %s",
                               strerror(-ret));
        return 0;
    }

    return gem->name;
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
