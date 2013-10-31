/**************************************************************************

libtbm-emulator

Copyright 2013 Samsung Electronics co., Ltd. All Rights Reserved.

Contact:
Stanislav Vorobiov <s.vorobiov@samsung.com>

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sub license, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice (including the
next paragraph) shall be included in all copies or substantial portions
of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

#ifndef _TBM_EMULATOR_LOG_H
#define _TBM_EMULATOR_LOG_H

typedef enum
{
    tbm_emulator_log_level_off = 0,
    tbm_emulator_log_level_error = 1,
    tbm_emulator_log_level_info = 2,
    tbm_emulator_log_level_debug = 3,
} tbm_emulator_log_level;

void tbm_emulator_log_event(tbm_emulator_log_level log_level,
                            const char *func,
                            int line,
                            const char *format, ...);

int tbm_emulator_log_is_debug_enabled();

#define TBM_EMULATOR_LOG_DEBUG(format, ...) \
    do { \
        if (tbm_emulator_log_is_debug_enabled()) { \
            tbm_emulator_log_event(tbm_emulator_log_level_debug, __FUNCTION__, __LINE__, format,##__VA_ARGS__); \
        } \
    } while(0)

#define TBM_EMULATOR_LOG_INFO(format, ...) tbm_emulator_log_event(tbm_emulator_log_level_info, __FUNCTION__, __LINE__, format,##__VA_ARGS__)
#define TBM_EMULATOR_LOG_ERROR(format, ...) tbm_emulator_log_event(tbm_emulator_log_level_error, __FUNCTION__, __LINE__, format,##__VA_ARGS__)

#endif
