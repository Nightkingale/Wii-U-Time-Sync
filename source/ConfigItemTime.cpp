#include "ConfigItemTime.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <wups.h>

void WUPSConfigItemTime_onDelete(void *context);

int32_t WUPSConfigItemTime_getCurrentValueDisplay(void *context, char *out_buf, int32_t out_size) {
    memset(out_buf, 0, out_size);
    return 0;
}

bool WUPSConfigItemTime_callCallback(void *context) {
    return false;
}

void WUPSConfigItemTime_onButtonPressed(void *context, WUPSConfigButtons buttons) {
}

bool WUPSConfigItemTime_isMovementAllowed(void *context) {
    return true;
}

int32_t WUPSConfigItemTime_getCurrentValueSelectedDisplay(void *context, char *out_buf, int32_t out_size) {
    memset(out_buf, 0, out_size);
    return 0;
}

void WUPSConfigItemTime_restoreDefault(void *context) {
}

void WUPSConfigItemTime_onSelected(void *context, bool isSelected) {
}

extern "C" ConfigItemTime
*WUPSConfigItemTime_AddToCategoryEx(WUPSConfigCategoryHandle cat, const char *configID, const char *displayName) {
    if (cat == 0) {
        return nullptr;
    }

    auto *item = (ConfigItemTime *) malloc(sizeof(ConfigItemTime));
    if (item == nullptr) {
        return nullptr;
    }

    WUPSConfigCallbacks_t callbacks = {
            .getCurrentValueDisplay         = &WUPSConfigItemTime_getCurrentValueDisplay,
            .getCurrentValueSelectedDisplay = &WUPSConfigItemTime_getCurrentValueSelectedDisplay,
            .onSelected                     = &WUPSConfigItemTime_onSelected,
            .restoreDefault                 = &WUPSConfigItemTime_restoreDefault,
            .isMovementAllowed              = &WUPSConfigItemTime_isMovementAllowed,
            .callCallback                   = &WUPSConfigItemTime_callCallback,
            .onButtonPressed                = &WUPSConfigItemTime_onButtonPressed,
            .onDelete                       = &WUPSConfigItemTime_onDelete};

    if (WUPSConfigItem_Create(&item->handle, configID, displayName, callbacks, item) < 0) {
        free(item);
        return nullptr;
    }

    if (WUPSConfigCategory_AddItem(cat, item->handle) < 0) {
        WUPSConfigItem_Destroy(item->handle);
        return nullptr;
    }
    return item;
}

void WUPSConfigItemTime_onDelete(void *context) {
    auto *item = (ConfigItemTime *) context;
    free(item);
}

extern "C" ConfigItemTime *WUPSConfigItemTime_AddToCategory(WUPSConfigCategoryHandle cat, const char *configID, const char *displayName) {
    return WUPSConfigItemTime_AddToCategoryEx(cat, configID, displayName);
}

extern "C" ConfigItemTime *WUPSConfigItemTime_AddToCategoryHandled(WUPSConfigHandle config, WUPSConfigCategoryHandle cat, const char *configID, const char *displayName) {
    ConfigItemTime *ret = WUPSConfigItemTime_AddToCategory(cat, configID,displayName);
    if (ret == NULL) {
        WUPSConfig_Destroy(config);
    }

    return ret;
}
