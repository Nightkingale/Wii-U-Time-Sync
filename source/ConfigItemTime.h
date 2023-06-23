#include <wups.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ConfigItemTime {
    WUPSConfigItemHandle handle;
} ConfigItemTime;

ConfigItemTime *WUPSConfigItemTime_AddToCategory(WUPSConfigCategoryHandle cat, const char *configID, const char *displayName);
ConfigItemTime *WUPSConfigItemTime_AddToCategoryHandled(WUPSConfigHandle config, WUPSConfigCategoryHandle cat, const char *configID, const char *displayName);

#ifdef __cplusplus
}
#endif
