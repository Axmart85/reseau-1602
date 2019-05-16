#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> /* gethostbyname */
#include <pthread.h>
#include <strings.h>
#include <poll.h>

#include "controler.h"
#include "fish.h"
#include "aquarium.h"
#include "vue.h"

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)

#define CRLF "\r\n"
#define MAX_CLIENTS 100

// #define PORT 1977
// #define MAX_WAITING_TIME 45
// #define UPDATE_INTERVAL 1

int PORT;
int MAX_WAITING_TIME;
int UPDATE_INTERVAL;

#define BUF_SIZE 1024

#define SUCCESS 1
#define ERROR -1

typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;

/**
 * Auxiliary function: split a string
 */
char **str_split(char *a_str, const char a_delim, int *wordCount) {

  char **result = 0;
  size_t count = 0;
  char *tmp = a_str;
  char *last_comma = 0;
  char delim[2];
  delim[0] = a_delim;
  delim[1] = 0;

  if (strlen(a_str) == 0){
    result = malloc(sizeof(char *));
    result[0] = malloc(sizeof(char*));
    strcpy(result[0], "");
    *wordCount = 0;
    return result;
  }

  /* Count how many elements will be extracted. */
  while (*tmp) {
    if (a_delim == *tmp) {
      count++;
      last_comma = tmp;
    }
    tmp++;
  }


  /* Add space for trailing token. */
  count += last_comma < (a_str + strlen(a_str) - 1);

  /* Add space for terminating null string so caller
     knows where the list of returned strings ends. */
  count++;

  result = malloc(sizeof(char *) * count);
  *wordCount = count - 1;

  if (result) {
    size_t idx = 0;
    char *token = strtok(a_str, delim);

    while (token) {
      assert(idx < count);
      *(result + idx++) = strdup(token);
      token = strtok(0, delim);
    }

    assert(idx == count - 1);
    *(result + idx) = 0;
  }

  return result;
}

int parseServerConfig() {

  FILE *f = fopen("controller.cfg", "r");
  if (f == NULL) {
    perror("Error opening configuration file");
    return EXIT_FAILURE;
  }

  char line[1024];
  int step = 0;

  while (!feof(f)) {
    if (!fgets(line, 1024, f)) {
      break;
    }

    if (line[0] == '#')
      continue;

    if (line[strlen(line)-1] == '\n'){
      line[strlen(line)-1] = '\0';
    }

    switch(step){
      case 0:
        if ((sscanf(line, "controller-port = %d", &PORT) == EOF)) {
          perror("Bad Configuration File Format: can't read port");
          fclose(f);
          return EXIT_FAILURE;
        }
        step++;
        break;

      case 1:
        if ((sscanf(line, "display-timeout-value = %d", &MAX_WAITING_TIME) == EOF)) {
          perror("Bad Configuration File Format: can't read timeout value");
          fclose(f);
          return EXIT_FAILURE;
        }
        step++;
        break;

      case 2:
        if ((sscanf(line, "fish-update-interval = %d", &UPDATE_INTERVAL) == EOF)) {
          perror("Bad Configuration File Format: can't read update interval value");
          fclose(f);
          return EXIT_FAILURE;
        }
        step++;
        break;

      default:
        fprintf(stderr, "Bad configuration file format\n");
        fclose(f);
        return EXIT_FAILURE;
        break;
    }
  }
  if (step != 3){
    fprintf(stderr, "Bad configuration file format\n");
    fclose(f);
    return EXIT_FAILURE;
  }
  fclose(f);
  return EXIT_SUCCESS;
}

static int init_connection(void) {
  SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
  SOCKADDR_IN sin = {0};

  if (sock == INVALID_SOCKET) {
    perror("socket()");
    exit(errno);
  }

  sin.sin_addr.s_addr = htonl(INADDR_ANY);
  sin.sin_port = htons(12345);
  sin.sin_family = AF_INET;

  int true = 1;
  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &true, sizeof(int));

  if (bind(sock, (SOCKADDR *)&sin, sizeof sin) == SOCKET_ERROR) {
    perror("bind()");
    exit(errno);
  }

  if (listen(sock, MAX_CLIENTS) == SOCKET_ERROR) {
    perror("listen()");
    exit(errno);
  }

  return sock;
}

static void end_connection(int sock) {
  closesocket(sock);
}

struct thread_assoc {
  pthread_t th;
  SOCKET sock;
};

void initThreadPool(struct thread_assoc *pool, int size) {
  for (int i = 0; i < size; i++) {
    pool[i].sock = -1;
  }
}

pthread_t *getAndSetAvailableThread(struct thread_assoc *pool, int size, SOCKET sock) {
  for (int i = 0; i < size; i++) {
    if (pool[i].sock == -1) {
      pool[i].sock = sock;
      return &(pool[i].th);
    }
  }
  return NULL;
}

void unsetThreadFromSock(struct thread_assoc *pool, int size, SOCKET sock) {
  for (int i = 0; i < size; i++) {
    if (pool[i].sock == sock) {
      pool[i].sock = -1;
      break;
    }
  }
}

static int myRead(SOCKET sock, char *buffer) {
  int n = 0;

  if ((n = recv(sock, buffer, BUF_SIZE - 1, 0)) < 0) {
    // perror("recv()");
    return -1;
  }

  buffer[n] = 0;
  return n;
}

static int myWrite(SOCKET sock, const char *buffer) {
  int n = 0;
  if ((n = send(sock, buffer, strlen(buffer), 0)) < 0) {
    // perror("send()");
    return -1;
  }
  return n;
}

void sendRefusal(SOCKET sock) {
  char buffer[BUF_SIZE];
  strcpy(buffer, "no greeting");
  myWrite(sock, buffer);
}

void freeStrSplitBuffer(char ** buffer, int wordCount){
  if (wordCount == 0){
    free(buffer[0]);
    free(buffer);
  } else {
    for (int i = 0; i < wordCount; i++){
      free(buffer[i]);
    }
    free(buffer);
  }
}

struct connectionHandlerData {
  struct aquarium *aq;
  SOCKET sock;
};

struct transferHandlerData {
  struct aquarium *aq;
  struct thread_assoc *pool;
  SOCKET sock;
};

int acceptVueRequest(SOCKET sock, struct aquarium * aq, struct client * client) {
  char buffer[BUF_SIZE];
  myRead(sock, buffer);
  int wordCount;

  char **words = str_split(buffer, ' ', &wordCount);

  if (strcmp(words[0], "hello") != 0) {
    sendRefusal(sock);
    freeStrSplitBuffer(words, wordCount);
    return 0;
  }

  if (!availableView(aq)) {
      sendRefusal(sock);
      freeStrSplitBuffer(words, wordCount);
      return 0;
  }

  client->sock = sock;

  if (wordCount == 1)
    client->vue = getAvailableView(aq, client);

  else if ((wordCount == 4)
	   && (strcmp(words[1], "in") == 0)
	   && (strcmp(words[2], "as") == 0)) {
    if (!isAvailableView(aq, words[3]))
      client->vue = getAvailableView(aq, client);
    else
      client->vue = getView(aq, words[3], client);
  }

  else {
    sendRefusal(sock);
    freeStrSplitBuffer(words, wordCount);
    return 0;
  }

  bzero(buffer, BUF_SIZE);
  strcpy(buffer, "greeting ");
  strcat(buffer, client->vue->vue_name);

  char width[10];
  char height[10];
  sprintf(width, " %d", client->vue->vue_width);
  strcat(buffer, width);
  sprintf(height, " %d", client->vue->vue_height);
  strcat(buffer, height);
  myWrite(sock, buffer);

  freeStrSplitBuffer(words, wordCount);
  return 1;
}

struct continuousUpdaterData {
  struct aquarium * aq;
  struct client * client;
  int * running;
};

void * updateContinuously(void * data){
  struct continuousUpdaterData *castedData = (struct continuousUpdaterData *)data;
  struct aquarium *aq = castedData->aq;
  struct client * client = castedData->client;

  while (*(castedData->running)){
    char buffer[1024];
    char buffer2[1024];
    strcpy(buffer2, "list");
    provide_data_vue(aq, client->vue, buffer, 0);
    strcat(buffer2, buffer);
    if (myWrite(client->sock, buffer2) == -1){
      break;
    }
    bzero(buffer, 1024);
    bzero(buffer2, 1024);
    sleep(UPDATE_INTERVAL);
  }
  pthread_exit(NULL);
}

struct timerData{
  int reader;
  volatile int * running;
};

void * startTimer(void * data){
  struct timerData * castedData = (struct timerData *) data;
  volatile int * running = castedData->running;
  int pipefd = castedData->reader;

  time_t start;
  int count = 0;

  while (*running) {
    time(&start);
    while (difftime(time(NULL), start) < 1)
      ;
    count++;

    if (poll(&(struct pollfd){ .fd = pipefd, .events = POLLIN }, 1, 0)==1) {
      while ((poll(&(struct pollfd){ .fd = pipefd, .events = POLLIN }, 1, 0)==1)){
        char ch;
        if (read(pipefd, &ch, 1) != 1){
          perror("read() : ");
        }
      }
      count = 0;
    }

    if (count > MAX_WAITING_TIME){
      if (poll(&(struct pollfd){ .fd = pipefd, .events = POLLIN }, 1, 0)!=1){
        break;
      }
    }
  }
  *running = 0;
  pthread_exit(NULL);
}

void manageClient(struct client *client, struct aquarium *aq) {
  pthread_t sendContinuousThread;
  pthread_t timerThread;

  int * sendContinuously = malloc(sizeof(int));
  *sendContinuously = 0;

  int * keepTimer = malloc(sizeof(int));
  *keepTimer = 1;

  int fd[2];
  if (pipe(fd) < 0){
    perror("pipe() : ");
    return ;
  }

  struct timerData * data = malloc(sizeof(struct timerData));
  data->reader = fd[0];
  data->running = keepTimer;

  pid_t timerPid;

  if ((timerPid = pthread_create(&timerThread, NULL, startTimer, data)) != 0){
    perror("Error starting timer thread");
  }

  struct timespec ts;
  ts.tv_sec = 0;
  ts.tv_nsec = 100 * 1000000;

  int breakOut = 0;

  while (*keepTimer == 1) {
    char buffer[BUF_SIZE];


    while (poll(&(struct pollfd){ .fd = client->sock, .events = POLLIN }, 1, 0)!=1){
      if (*keepTimer == 0){
        breakOut = 1;
        break;
      }
      nanosleep(&ts, NULL);
    }

    if (breakOut){
      break;
    }

    myRead(client->sock, buffer);

    char ch = 'A';
    if (write(fd[1], &ch, 1) != 1){
      perror("write() : ");
    }

    int wordCount;

    char **words = str_split(buffer, ' ', &wordCount);


    bzero(buffer, BUF_SIZE);

    if ((wordCount == 2) && (strcmp(words[0], "log") == 0) && (strcmp(words[1], "out") == 0)) {
      strcpy(buffer, "bye");
      myWrite(client->sock, buffer);
      freeStrSplitBuffer(words, wordCount);
      break;
    } else  if ((wordCount == 2) && (strcmp(words[0], "ping") == 0)) {
      if (*sendContinuously != 1) {
        strcpy(buffer, "pong ");
        strcat(buffer, words[1]);
        myWrite(client->sock, buffer);
      } else {
        char buffer2[1024];
        strcpy(buffer2, "list");
        provide_data_vue(aq, client->vue, buffer, 0);
        strcat(buffer2, buffer);
        myWrite(client->sock, buffer2);
      }
    } else if ((wordCount == 6) && (strcmp(words[0], "addFish") == 0) &&
            (strcmp(words[2], "at") == 0)) {
      int posx, posy, sizex, sizey;

      sscanf(words[3], "%dx%d,", &posx, &posy);
      sscanf(words[4], "%dx%d,", &sizex, &sizey);

      struct position viewPos = {posx, posy};
      struct position aqPos = viewToAqCoord(client->vue, viewPos);
      int coordOK = (aqPos.x >=0) && (aqPos.y >= 0);

      if (coordOK && add_fish(aq, words[1], words[5], aqPos.x, aqPos.y, sizex, sizey)){
        strcpy(buffer, "OK");
      } else {
        strcpy(buffer, "NOK");
      }

      myWrite(client->sock, buffer);
    } else if ((wordCount == 2) && (strcmp(words[0], "delFish") == 0)) {

      if (del_fish(aq, words[1])){
        strcpy(buffer, "OK");
      } else {
        strcpy(buffer, "NOK");
      }

      myWrite(client->sock, buffer);
    } else if ((wordCount == 2) && (strcmp(words[0], "startFish") == 0)) {

      if (start_fish(aq, words[1])){
        strcpy(buffer, "OK");
      } else {
        strcpy(buffer, "NOK");
      }

      myWrite(client->sock, buffer);
    } else if ((wordCount == 1) && (strcmp(words[0], "getFishes") == 0)) {

      char buffer2[1024];
      strcpy(buffer2, "list");
      provide_data_vue(aq, client->vue, buffer, 0);
      strcat(buffer2, buffer);
      myWrite(client->sock, buffer2);

    } else if ((wordCount == 1) && (strcmp(words[0], "status") == 0)) {

      char buffer2[1024];
      strcpy(buffer2, "stat");
      provide_data_vue(aq, client->vue, buffer, 1);
      strcat(buffer2, buffer);
      myWrite(client->sock, buffer2);

    } else if ((wordCount == 1) && (strcmp(words[0], "getFishesContinuously") == 0) && (*sendContinuously != 1)) {

      *sendContinuously = 1;

      struct continuousUpdaterData * data = malloc(sizeof(struct continuousUpdaterData));
      data->aq = aq;
      data->client = client;
      data->running = sendContinuously;
      if (pthread_create(&sendContinuousThread, NULL, updateContinuously, data) != 0){
        perror("Error starting continuous update thread");
        break;
      }

    } else if ((wordCount == 1) && (strcmp(words[0], "stopContinuous") == 0)) {

      *sendContinuously = 0;
      strcpy(buffer, "OK");
      myWrite(client->sock, buffer);

    } else {

      strcpy(buffer, "NOK1");
      myWrite(client->sock, buffer);

    }
    freeStrSplitBuffer(words, wordCount);
  }
  *sendContinuously = 0;
  *keepTimer = 0;
}

void *transferHandler(void *data) {
  struct transferHandlerData *thd = (struct transferHandlerData *)data;
  struct aquarium *aq = thd->aq;
  struct thread_assoc *th_pool = thd->pool;
  SOCKET sock = thd->sock;
  struct client client;

  if (acceptVueRequest(sock, aq, &client)) {
    manageClient(&client, aq);
    client.vue->monitored_by = NULL;
  };
  unsetThreadFromSock(th_pool, MAX_CLIENTS, sock);
  end_connection(sock);
  pthread_exit(NULL);
}

void *connectionHandler(void *data) {
  struct thread_assoc *thread_pool = malloc(MAX_CLIENTS * sizeof(struct thread_assoc));
  struct connectionHandlerData *chd = (struct connectionHandlerData *)data;
  struct aquarium *aq = chd->aq;
  int listen_socket = chd->sock;
  static struct sockaddr_in addr_client;
  SOCKET service_socket;
  socklen_t addr_len;

  initThreadPool(thread_pool, MAX_CLIENTS);
  while (1) {
    addr_len = sizeof(struct sockaddr_in);
    service_socket = accept(listen_socket, (struct sockaddr *)&addr_client, &addr_len);
    if (service_socket == -1) {
      perror("Error accepting socket");
      exit(1);
    }

    struct transferHandlerData *thd = malloc(sizeof(struct transferHandlerData));
    thd->aq = aq;
    thd->pool = thread_pool;
    thd->sock = service_socket;

    pthread_t *serviceThread = getAndSetAvailableThread(thread_pool, MAX_CLIENTS, service_socket);

    if (pthread_create(serviceThread, NULL, transferHandler, thd) != 0) {
      unsetThreadFromSock(thread_pool, MAX_CLIENTS, service_socket);
      perror("Error starting transfer handler");
    }
  }
  free(thread_pool);
  pthread_exit(NULL);
}

void *aquariumRefresher(void * data) {
  struct aquarium * aq = (struct aquarium *) data;
  time_t t;
  srand((unsigned) time(&t));
  while (1){
    move_fishes(aq);
    sleep(aq->timelaps);
  }
  pthread_exit(NULL);
}

int main(int argc, char const *argv[]) {
  if (parseServerConfig() != EXIT_SUCCESS){
    return -1;
  }

  struct aquarium *aq = malloc(sizeof(struct aquarium));
  (void) argc;
  (void) argv;

  initAquarium(aq);

  int keepConnectionHandler = 1;

  SOCKET sock = init_connection();
  struct connectionHandlerData chd;
  chd.aq = aq;
  chd.sock = sock;

  pthread_t connectionThread;

  if (pthread_create(&connectionThread, NULL, connectionHandler, &chd)) {
    perror("Error starting TCP connection handler thread");
    return EXIT_FAILURE;
  }

  pthread_t refreshThread;

  if (pthread_create(&refreshThread, NULL, aquariumRefresher, aq)) {
    perror("Error starting aquarium refreshment thread");
    return EXIT_FAILURE;
  }

  char input[256];
  while (1) {
    printf("$ ");
    fgets(input, 256, stdin);

    if ((strlen(input) > 0) && (input[strlen(input) - 1] == '\n'))
      input[strlen(input) - 1] = '\0';

    int wordCount;
    char **words = str_split(input, ' ', &wordCount);

    if (wordCount == 0){
      // freeStrSplitBuffer(words, wordCount);
    }

    if (strcmp(words[0], "load") == 0 && (wordCount == 2)) {
      char name[256] = "";
      strcat(name, words[1]);
      strcat(name, ".txt");
      int status = load_aquarium(aq, name);

      if (status == EXIT_SUCCESS)
        printf("\t-> aquarium loaded (%d display view)!\n", aq->size_vues);
    }

    else if (strcmp(words[0], "show") == 0 && (wordCount == 2)) {
      if (strcmp(words[1], "aquarium") == 0)
        show_aquarium(aq);
    }

    else if (strcmp(words[0], "add") == 0 && (wordCount == 4) && strcmp(words[1], "view") == 0) {
      int vueX, vueY, vueWidth, vueHeight;
      sscanf(words[3], "%dx%d+%d+%d", &vueX, &vueY, &vueWidth, &vueHeight);

      struct vue new_view;
      new_view = initVue(words[2], vueX, vueY, vueHeight, vueWidth);
      int status = add_view(aq, new_view);

      if (status == EXIT_SUCCESS)
        printf("\t-> view added\n");

      if (status == -1) {
        perror("Error allocating memory");
        freeStrSplitBuffer(words, wordCount);
        break;
      }

      if (status == 1) {
        fprintf(stderr, "Vue size and position do not match aquarium size\n");
      }
    }

    else if (strcmp(words[0], "del") == 0 && (wordCount == 3) && strcmp(words[1], "view") == 0) {
      if (del_view(aq, words[2]) == EXIT_SUCCESS)
        printf("\t-> view %s deleted\n", words[2]);
    }

    else if ((strcmp(words[0], "save") == 0) && (wordCount == 2)) {
      char name[256] = "";
      strcat(name, words[1]);
      strcat(name, ".txt");

      if (save_aquarium(aq, name) == EXIT_SUCCESS)
        printf("\t-> Aquarium saved (%d display view)\n", aq->size_vues);
    }

    else if (strcmp(words[0], "exit") == 0) {
      end_connection(sock);
      keepConnectionHandler = 0;
      freeStrSplitBuffer(words, wordCount);
      break;
    }

    else
      printf("Please enter a valid input\n");

    freeStrSplitBuffer(words, wordCount);
  }
  free_aquarium(aq);
  return 0;
}
