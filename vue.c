#include <string.h>

#include "vue.h"

/**
 * Copy the view src in dest
 */
void copy_view(struct vue *dest, const struct vue src) {
  strcpy(dest->vue_name, src.vue_name);
  dest->vue_x = src.vue_x;
  dest->vue_y = src.vue_y;
  dest->vue_width = src.vue_width;
  dest->vue_height = src.vue_height;
  dest->monitored_by = src.monitored_by;
}

struct vue initVue(char *name, int vueX, int vueY, int vueHeight, int vueWidth) {
  struct vue v;
  v.vue_x = vueX;
  v.vue_y = vueY;
  v.vue_height = vueHeight;
  v.vue_width = vueWidth;
  strcpy(v.vue_name, name);
  v.monitored_by = NULL;

  return v;
}

void shift_left_vues(struct vue *begin, struct vue *end) {
  struct vue *p = NULL;
  for (p = begin; p < end; p++) {
    *p = *(p + 1);
  }
}