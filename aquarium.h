#ifndef AQUARIUM_H
#define AQUARIUM_H

#include "fish.h"
#include "vue.h"

struct aquarium{
  int width;
  int height;
  struct vue* vues;
  int size_vues;
  int vues_capacity;
  struct fish* fishes;
  int size_fishes;
  int fishes_capacity;
  float timelaps;
};

void initAquarium(struct aquarium *);
int load_aquarium(struct aquarium *, char* file_name);
void show_aquarium(const struct aquarium* aq);
int add_view(struct aquarium* aq, struct vue v);
int del_view(struct aquarium* aq, char *name);
int save_aquarium(const struct aquarium* aq, char* file_name);
int add_fish(struct aquarium* aq, char* name, char* mobility, int positionx, int positiony, int sizex, int sizey);
int del_fish(struct aquarium* aq, char *);
int start_fish(struct aquarium *, char *);
void move_fishes(struct aquarium* aq);
int fish_inside_vue(const struct fish *, const struct vue *);
void status(struct aquarium*);
void free_aquarium(struct aquarium *);
int isAvailableView(struct aquarium *, char *);
int availableView(struct aquarium *);
struct vue * getAvailableView(struct aquarium *, struct client *);
struct vue * getView(struct aquarium *, char *, struct client *);
struct position viewToAqCoord(const struct vue *, struct position);
struct position aqToViewCoord(const struct vue *, struct position);
void provide_data_vue(const struct aquarium *,const struct vue *, char *, int);


#endif
