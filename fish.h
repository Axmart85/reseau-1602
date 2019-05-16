#ifndef FISH_H
#define FISH_H

struct position{
  int x;
  int y;
};

struct fish{
  struct position pos;
  struct position dest;
  int timetoarrived;
  int fish_width;
  int fish_height;
  int started;
  int id_fish;
  char name[32];
  char mobility[32];
};

void shift_left_fishes(struct fish *, struct fish *);
void init_fish(struct fish *, char *, char *, int, int, int, int);

#endif
