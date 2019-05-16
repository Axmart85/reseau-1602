#ifndef VUE_H
#define VUE_H

struct vue{
  int vue_x;
  int vue_y;
  int vue_width;
  int vue_height;
  char vue_name[256];
  struct client * monitored_by;
};

struct vue initVue(char *, int, int, int, int);
void copy_view(struct vue *, const struct vue);
void shift_left_vues(struct vue *, struct vue *);

#endif
