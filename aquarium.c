#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "aquarium.h"

int fish_id = 0;

/**
 * Add the view v in the structure aq
 */
int add_view(struct aquarium *aq, struct vue v) {
  if ((v.vue_x + v.vue_width > aq->width) || (v.vue_y + v.vue_height > aq->height))
    return 1;

  if (aq->size_vues < aq->vues_capacity)
    copy_view(&(aq->vues[aq->size_vues]), v);

  else {
    aq->vues_capacity = aq->vues_capacity * 2;
    aq->vues = realloc(aq->vues, aq->vues_capacity);
    if (!aq->vues)
      return -1;
    copy_view(&(aq->vues[aq->size_vues]), v);
  }

  aq->size_vues++;
  return 0;
}

/**
 * Count the number of lines in the file p
 */
int countlines(FILE *fp) {
  // count the number of lines in the file called filenames
  int ch = 0;
  int lines = 0;

  if (fp == NULL)
    return 0;

  lines++;
  ch = fgetc(fp);
  while (ch != EOF) {
    if (ch == '\n')
      lines++;
    ch = fgetc(fp);
  }

  return lines;
}

/**
 * Read the configuration file
 */
int parseConfig(struct aquarium *aq, char *file_name) {
  FILE *f = fopen(file_name, "r");
  if (f == NULL) {
    perror("Error opening file");
    return EXIT_FAILURE;
  }

  f = fopen(file_name, "r");
  int lines = countlines(f);

  aq->size_vues = lines - 2;
  aq->vues_capacity = aq->size_vues * 2;
  aq->vues = malloc(aq->vues_capacity * sizeof(struct vue));
  rewind(f);

  if ((fscanf(f, "%dx%d", &(aq->width), &(aq->height)) == EOF)) {
    perror("Bad File Format: can't read line 1");
    return EXIT_FAILURE;
  }

  char name[256];
  int vueX, vueY, vueHeight, vueWidth, i;
  for (i = 2; fscanf(f, "%s %dx%d+%d+%d", name,
		     &vueX, &vueY, &vueWidth, &vueHeight) != EOF; i++) {
    aq->vues[i - 2] = initVue(name, vueX, vueY, vueHeight, vueWidth);
  }

  if (i < lines - 1) {
    perror("Error parsing file");
    return EXIT_FAILURE;
  }

  fclose(f);
  return EXIT_SUCCESS;
}

/**
 * Delete the view from the aquarium aq
 */
int del_view(struct aquarium *aq, char *name) {
  int index = -1;
  for (int i = 0; i < aq->size_vues; i++)   {
    if (strcmp(aq->vues[i].vue_name, name) == 0)
      index = i;
  }

  if (index != -1 && index < aq->size_vues) {
    shift_left_vues(aq->vues + index, aq->vues + aq->size_vues - 1);
    aq->size_vues--;
    return EXIT_SUCCESS;
  }

  return EXIT_FAILURE;
}

/**
 * Add a fish in the aquarium aq
 */
int add_fish(struct aquarium *aq, char *name, int positionx, int positiony, int sizex, int sizey) {
  if (sizex <= 0 || sizey <= 0){
    return 0;
  }

  for (int i = 0; i < aq->size_fishes; i++) {
    if (strcmp(aq->fishes[i].name, name) == 0)
      return 0;
  }

  struct fish f;
  init_fish(&f, name, positionx, positiony, sizex, sizey);
  f.id_fish = fish_id++;

  if (aq->size_fishes < aq->fishes_capacity)
    aq->fishes[aq->size_fishes] = f;
  else {
    aq->fishes_capacity = aq->fishes_capacity * 2;
    aq->fishes = realloc(aq->fishes, aq->fishes_capacity);
    aq->fishes[aq->size_fishes] = f;
  }

  aq->size_fishes++;

  return 1;
}


/**
 * Delete the fish with the name "name" from the aquarium aq
 */
int del_fish(struct aquarium *aq, char * name) {
  int index = -1;
  for (int i = 0; i < aq->size_fishes; i++) {
    if (strcmp(aq->fishes[i].name, name) == 0){
      index = i;
      break;
    }
  }

  if (index != -1 && index < aq->size_fishes) {
    shift_left_fishes(aq->fishes + index, aq->fishes + aq->size_fishes - 1);
    aq->size_fishes--;
    return 1;
  }

  return 0;
}

/**
 * Starts the fish with the name "name" from the aquarium aq
 */
int start_fish(struct aquarium *aq, char * name) {
  for (int i = 0; i < aq->size_fishes; i++) {
    if (strcmp(aq->fishes[i].name, name) == 0){
      aq->fishes[i].started = 1;
      return 1;
    }
  }
  return 0;
}

/**
 * Return 1 if the position pos is inside the view v, 0 if not
 */
int pos_inside_vue(const struct position pos, const struct vue * v) {
  return pos.x >= v->vue_x && pos.y >= v->vue_y && pos.x < v->vue_x + v->vue_height && pos.y < v->vue_y + v->vue_width;
}

/**
 * Return 1 if the fish f is inside the view v, 0 if not
 */
int fish_inside_vue(const struct fish * f, const struct vue * v) {
  struct position p;
  for (int i = f->pos.x; i <= f->pos.x + f->fish_width; i+= f->fish_width) {
    for (int j = f->pos.y; j <= f->pos.y + f->fish_height; j+= f->fish_height) {
      p.x = i;
      p.y = j;
      if (pos_inside_vue(p,v)){
        return 1;
      }
    }
  }
  return 0;
}

/**
 * Return 1 if the destinatination of the fish f is inside the view v, 0 if not
 */
int fish_dest_inside_vue(const struct fish * f, const struct vue * v) {
  struct position p;
  for (int i = f->dest.x; i <= f->dest.x + f->fish_width; i+= f->fish_width) {
    for (int j = f->dest.y; j <= f->dest.y + f->fish_height; j+= f->fish_height) {
      p.x = i;
      p.y = j;
      if (pos_inside_vue(p,v)){
        return 1;
      }
    }
  }
  return 0;
}

/**
 * Return 1 if the way of the fish f is inside the view v, 0 if not
 */
int fish_way_inside_vue(const struct fish * f, const struct vue * v) {
  struct position p1;
  struct position p2;
  struct position p;
  int nbPos = 10;
  int k = f->pos.x;
  for (int i = f->dest.x ; i <= f->dest.x + f->fish_width; i+= f->fish_width) {
    int l = f->pos.y;
    for (int j = f->dest.y; j <= f->dest.y + f->fish_height; j+= f->fish_height) {
      p1.x = k;
      p1.y = l;
      p2.x = i;
      p2.y = j;
      float stepx = (float)(p2.x - p1.x) / nbPos;
      float stepy = (float)(p2.y - p1.y) / nbPos;
      for (int n = 1; n < nbPos-1; n++) {
        p.x = (int)(p1.x + n * stepx);
        p.y = (int)(p1.y + n * stepy);
        if (pos_inside_vue(p,v)){
          return 1;
        }
      }
      l+= f->fish_height;
    }
    k += f->fish_width;
  }
  return 0;
}

void newfishdestination(struct aquarium* aq, struct fish* f){
  f->pos.x = f->dest.x;
  f->pos.y = f->dest.y;

  f->dest.x = rand()%aq->height;
  f->dest.y = rand()%aq->width;
}

void move_fishes(struct aquarium* aq) {
  for (int i = 0; i < aq->size_fishes; i++) {
    if (aq->fishes[i].started) {
      // printf("%f, pos %d/%d\n", aq->timelaps, aq->fishes[i].pos.x, aq->fishes[i].pos.y);
      newfishdestination(aq, &aq->fishes[i]);
    }
  }
}

/**
 * Initialize the values of the aquarium aq
 */
void initAquarium(struct aquarium *aq) {
  aq->width = 0;
  aq->height = 0;
  aq->vues = NULL;
  aq->size_vues = 0;
  aq->vues_capacity = 0;
  aq->fishes = NULL;
  aq->size_fishes = 0;
  aq->fishes_capacity = 0;
  aq->timelaps = 5.0;
}


/**
 * Load an aquarium from the file to a structure
 */
int load_aquarium(struct aquarium *aq, char *file_name) {
  if (parseConfig(aq, file_name) != EXIT_SUCCESS)
    return EXIT_FAILURE;

  aq->fishes_capacity = 4;
  aq->size_fishes = 0;
  aq->fishes = malloc(aq->fishes_capacity * sizeof(struct fish));

  if (!aq->fishes)
    return EXIT_FAILURE;

  return EXIT_SUCCESS;
}


/**
 * Save the aquarium aq in the file
 */
int save_aquarium(const struct aquarium *aq, char *file_name) {
  FILE *f = fopen(file_name, "w+");
  if (f == NULL) {
    perror("Error opening file");
    return EXIT_FAILURE;
  }

  fprintf(f, "%dx%d\n", aq->width, aq->height);
  for (int i = 0; i < aq->size_vues; i++) {
    fprintf(f, "%s %dx%d+%d+%d\n", aq->vues[i].vue_name, aq->vues[i].vue_x, aq->vues[i].vue_y, aq->vues[i].vue_width, aq->vues[i].vue_height);
  }

  fclose(f);
  return EXIT_SUCCESS;
}


/**
 * Free the aquarium
 */
void free_aquarium(struct aquarium *aq) {
  free(aq->fishes);
  free(aq->vues);
  free(aq);
}


/**
 * Display the configuration of the aquarium
 */
void show_aquarium(const struct aquarium *aq) {
  printf("%dx%d\n", aq->width, aq->height);

  for (int i = 0; i < aq->size_vues; i++) {
    printf("N%d %dx%d+%d+%d\n", i + 1, aq->vues[i].vue_x, aq->vues[i].vue_y, aq->vues[i].vue_width, aq->vues[i].vue_height);
  }
}


/**
 * Display the number of views and the fishes
 */
void status(struct aquarium *aq) {
  printf("%d vues connectees\n", aq->size_vues);

  for (int i = 0; i < aq->size_fishes; i++) {
    printf("Fish %d at %d, %d\n", aq->fishes[i].id_fish, aq->fishes[i].pos.x, aq->fishes[i].pos.y);
  }
}

int isAvailableView(struct aquarium * aq, char * vue_name) {
  for (int i = 0; i < aq->size_vues; i ++){
    if (strcmp(aq->vues[i].vue_name, vue_name) == 0){
      return (aq->vues[i].monitored_by == NULL);
    }
  }
  return 0;
}

int availableView(struct aquarium * aq) {
  for (int i = 0; i < aq->size_vues; i ++){
    if (aq->vues[i].monitored_by == NULL){
      return 1;
    }
  }
  return 0;
}

struct vue * getAvailableView(struct aquarium * aq, struct client * c) {
  for (int i = 0; i < aq->size_vues; i ++){
    if (!aq->vues[i].monitored_by){
      aq->vues[i].monitored_by = c;
      return &(aq->vues[i]);
    }
  }
  return NULL;
}

struct vue * getView(struct aquarium * aq, char * vue_name, struct client * c) {
  for (int i = 0; i < aq->size_vues; i ++){
    if (strcmp(aq->vues[i].vue_name, vue_name) == 0){
      aq->vues[i].monitored_by = c;
      return &(aq->vues[i]);
    }
  }
  return NULL;
}

/**
 * Return the coordinate in the aquarium referential, given the coordinates in the view referential
 */
struct position viewToAqCoord(const struct vue * v, struct position viewCoord){
  struct position aqCoord;
  aqCoord.x = v->vue_x + viewCoord.x;
  aqCoord.y = v->vue_y + viewCoord.y;
  return aqCoord;
}

/**
 * Return the coordinate in the view referential, given the coordinates in the aquarium referential
 */
struct position aqToViewCoord(const struct vue * v, struct position aqCoord){
  struct position viewCoord;
  viewCoord.x = aqCoord.x - v->vue_x;
  viewCoord.y = aqCoord.y - v->vue_y;
  return viewCoord;
}

/**
 * Display the information of the vue v
 */
void provide_data_vue(const struct aquarium *aq,const struct vue * v, char * buffer, int giveStatus) {
  for (int i = 0; i < aq->size_fishes; i++) {
    if (fish_inside_vue(&(aq->fishes[i]), v) || fish_dest_inside_vue(&(aq->fishes[i]), v) || fish_way_inside_vue(&(aq->fishes[i]), v)) {
      struct position aqPos = {aq->fishes[i].pos.x, aq->fishes[i].pos.y};
      struct position viewPos = aqToViewCoord(v, aqPos);
      struct position viewDest;
      if (aq->fishes[i].started){
        viewDest = aqToViewCoord(v, aq->fishes[i].dest);
      } else {
        viewDest = viewPos;
      }
      char string[1024];
      sprintf(string, " [%s at %dx%d,%dx%d,%d]", aq->fishes[i].name, viewPos.x, viewPos.y, viewDest.x, viewDest.y, giveStatus? aq->fishes[i].started : aq->fishes[i].timetoarrived);
      strcat(buffer, string);
    }
  }
}
// addFish PoissonNain at 10x10, 60x60, RandomWayPoint
