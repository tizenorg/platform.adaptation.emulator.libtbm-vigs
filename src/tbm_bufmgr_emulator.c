#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <tbm_bufmgr.h>
#include <tbm_bufmgr_backend.h>
#include "vigs.h"
#include "tbm_emulator_log.h"
#include <string.h>

static void tbm_bufmgr_emulator_deinit(void *priv)
{
    TBM_EMULATOR_LOG_DEBUG("enter");
}

static int tbm_bufmgr_emulator_bo_size(tbm_bo bo)
{
    TBM_EMULATOR_LOG_DEBUG("enter");
    return 0;
}

static void *tbm_bufmgr_emulator_bo_alloc(tbm_bo bo, int size, int flags)
{
    TBM_EMULATOR_LOG_DEBUG("enter");
    return NULL;
}

static void tbm_bufmgr_emulator_bo_free(tbm_bo bo)
{
    TBM_EMULATOR_LOG_DEBUG("enter");
}

static void *tbm_bufmgr_emulator_bo_import(tbm_bo bo, unsigned int key)
{
    TBM_EMULATOR_LOG_DEBUG("enter");
    return NULL;
}

static unsigned int tbm_bufmgr_emulator_bo_export(tbm_bo bo)
{
    TBM_EMULATOR_LOG_DEBUG("enter");
    return 0;
}

static tbm_bo_handle tbm_bufmgr_emulator_bo_get_handle(tbm_bo bo, int device)
{
    tbm_bo_handle handle;
    handle.ptr = NULL;
    TBM_EMULATOR_LOG_DEBUG("enter");
    return handle;
}

static tbm_bo_handle tbm_bufmgr_emulator_bo_map(tbm_bo bo, int device, int opt)
{
    tbm_bo_handle handle;
    handle.ptr = NULL;
    TBM_EMULATOR_LOG_DEBUG("enter");
    return handle;
}

static int tbm_bufmgr_emulator_bo_unmap(tbm_bo bo)
{
    TBM_EMULATOR_LOG_DEBUG("enter");
    return 0;
}

static int tbm_bufmgr_emulator_bo_cache_flush(tbm_bo bo, int flags)
{
    TBM_EMULATOR_LOG_DEBUG("enter");
    return 0;
}

static int tbm_bufmgr_emulator_bo_get_global_key(tbm_bo bo)
{
    TBM_EMULATOR_LOG_DEBUG("enter");
    return 0;
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
