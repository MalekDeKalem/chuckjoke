#include <stdio.h>
#include <curl/curl.h>
#include <string.h>
#include <stdlib.h>
#include <argp.h>
#include <ncurses.h>
#include "json-parser/json.h"


#define RANDOM_URL "https://api.chucknorris.io/jokes/random"
#define CATEGORIES_URL "https://api.chucknorris.io/jokes/categories"
#define USERAGENT "libcurl/1.0"
#define CATEGORY_LIST_BUFSIZE 300
#define URL_BUFSIZE 64



#define KEY_Q 113


typedef struct {
  char* memory;
  size_t size;
} MemBuffer;

static char args_doc[] = "chuckjoke [OPTIONS]";
static char doc[] = "chuckjoke -- a cli tool to get chucknorris jokes from the official chucknorris api";

static struct argp_option options[] = {
  {"search", 's', "SEARCH", 0, "Search for a joke", 0},
  {"category", 'c', "CATEGORY", 0, "Get a random joke from a category", 0},
  {"list", 'l', 0, 0, "Get list of categories", 0},
  {0, 0, 0, 0, 0, 0}
};

struct arguments {
  int list;
  char* category; /* Argument for -c */
  char* search; /* Argument for -s */
};

static error_t parse_opt(int key, char* arg, struct argp_state* state) {
  struct arguments* arguments = state->input;

  switch (key) {
    case 'l':
      arguments->list = 1;
      break;
    case 'c':
      arguments->category = arg;
      break;
    case 's':
      arguments->search = arg;
      break;
    default:
      return ARGP_ERR_UNKNOWN;
  }

  return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc, 0, 0, 0};

static char categoryList[CATEGORY_LIST_BUFSIZE];

static size_t writeCurlResponse(char* contents, size_t size, size_t nmemb, void* userp) {
  size_t realsize = size * nmemb;
  MemBuffer* mem = (MemBuffer *) userp;
  char* ptr = realloc(mem->memory, mem->size + realsize + 1);
  if (!ptr) {
    fprintf(stderr, "Not enough memory\n");
    return 0;
  }
  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;
  return realsize;
}

static WINDOW *createWindow(int height, int width, int starty, int startx) {
  WINDOW* localWin = newwin(height, width, starty, startx);
  box(localWin, 0, 0);
  wrefresh(localWin);
  return localWin;
}

static void destroyWindow(WINDOW* localWin) {
  wborder(localWin, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
  wrefresh(localWin);
  delwin(localWin);
}


static char* getRandom(CURL* curl, CURLcode* res, MemBuffer *buf) {
  curl_easy_setopt(curl, CURLOPT_URL, RANDOM_URL);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCurlResponse);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)buf);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, USERAGENT);
  *res = curl_easy_perform(curl);

  if (*res != CURLE_OK) {
    fprintf(stderr, "Performing curl failed %s\n", curl_easy_strerror(*res));
  } else {
    printf("%lu bytes retrieved\n", (unsigned long) buf->size);
    buf->memory[buf->size] = '\0';
  }

  json_value *val = json_parse(buf->memory, buf->size);
  char* joke = val->u.object.values[6].value->u.string.ptr;
  curl_easy_cleanup(curl);
  return joke;
}

static char* getRandomFromCategory(CURL* curl, CURLcode* res, MemBuffer *buf, char* category) {
  char urlBuffer[URL_BUFSIZE];
  snprintf(urlBuffer, URL_BUFSIZE, "https://api.chucknorris.io/jokes/random?category=%s", category);
  printf("%s\n", urlBuffer);
  curl_easy_setopt(curl, CURLOPT_URL, urlBuffer);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCurlResponse);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) buf);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, USERAGENT);
  *res = curl_easy_perform(curl);

  if (*res != CURLE_OK) {
    fprintf(stderr, "Performing curl failed %s\n", curl_easy_strerror(*res));
  } else {
    printf("%lu bytes retrieved\n", (unsigned long) buf->size);
    buf->memory[buf->size] = '\0';
  }

  json_value *val = json_parse(buf->memory, buf->size);
  char* joke = val->u.object.values[6].value->u.string.ptr;
  curl_easy_cleanup(curl);


  return joke;
}

static char* getCategoryList(CURL* curl, CURLcode* res, MemBuffer* buf) {
  curl_easy_setopt(curl, CURLOPT_URL, CATEGORIES_URL);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCurlResponse);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) buf);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, USERAGENT);
  *res = curl_easy_perform(curl);

  if (*res != CURLE_OK) {
    fprintf(stderr, "Performing curl failed %s\n", curl_easy_strerror(*res));
  } else {
    printf("%lu bytes retrieved\n", (unsigned long) buf->size);
    buf->memory[buf->size] = '\0';
  }

  json_value *val = json_parse(buf->memory, buf->size);
  size_t length = val->u.array.length;
  for (size_t i = 0; i < length; i++) {
    char* category = val->u.array.values[i][0].u.string.ptr;
    strcat(categoryList, category);
    strcat(categoryList, "\n");
  }

  curl_easy_cleanup(curl);
  return categoryList;
}

void keyHandler(int key) {
  switch (key) {
    case KEY_RESIZE:
      break;
    default:
      printf("Key: %d\n", key);
  }
}


int main(int argc, char** argv) {
  CURL* curl;
  CURLcode res;

  MemBuffer buf;

  struct arguments arguments;


  res = curl_global_init(CURL_GLOBAL_DEFAULT);
  if (res != CURLE_OK) {
    return (int)res;
  }

  curl = curl_easy_init();
  buf.memory = malloc(1);
  buf.size = 0;



  arguments.category = "";
  arguments.search = "";
  arguments.list = 0;


  argp_parse(&argp, argc, argv, 0, 0, &arguments);


  if (curl) {
    if (strlen(arguments.category) > 0) {
      char* joke = getRandomFromCategory(curl, &res, &buf, arguments.category);
      printf("%s\n", joke);
    } else if (strlen(arguments.search) > 0) {
      int startx, starty, width, height;
      initscr();
      height = LINES;
      width = COLS;
      startx = 0;
      starty = 0;
      int c;
      WINDOW* win = createWindow(height, width, starty, startx);
      wborder(win, '|', '|', '-', '-', '+', '+', '+', '+');
      while ((c = wgetch(win)) != KEY_Q) {
        keyHandler(c);
      }
      destroyWindow(win);
      endwin();
      printf("Starting search\n");
    } else if (arguments.list) {
      getCategoryList(curl, &res, &buf);
      printf("%s\n", categoryList);
    } else {
      char* joke = getRandom(curl, &res, &buf);
      printf("%s\n", joke);
    }
  }



  free(buf.memory);
  curl_global_cleanup();
  return 0;
}
