#include "libglace-private.h"

GlaceManager* glace_manager_new() {
    return g_object_new(GLACE_TYPE_MANAGER, NULL);
}
