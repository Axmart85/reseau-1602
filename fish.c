#include <string.h>
#include <stdlib.h>

#include "fish.h"

void shift_left_fishes(struct fish *begin, struct fish *end) {
  struct fish *p = NULL;
  for (p = begin; p < end; p++) {
    *p = *(p + 1);
  }
}

void init_fish(struct fish *res, char *fname, char *fmobility, int positionx, int positiony, int sizex, int sizey) {
  strcpy(res->name, fname);
  strcpy(res->mobility, fmobility);
  res->pos.x = positionx;
  res->pos.y = positiony;
  res->dest.x = positionx;
  res->dest.y = positiony;
  res->fish_height = sizex;
  res->fish_width = sizey;
  res->timetoarrived = 5;
  res->started = 0;
}
