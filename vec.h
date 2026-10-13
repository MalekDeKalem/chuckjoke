#ifndef VEC_H
#define VEC_H

#include <stdlib.h>


// Declarations 

typedef struct {
  int x;
  int y;
} Point;

typedef struct {
  int* items;
  size_t size;
  size_t capacity;
} Vec;

typedef struct {
  Point* points;
  size_t size;
  size_t capacity;
} PointVec;

Vec* createVec();
void pushVecItem(Vec* vec, int item);
void pushPointVecItem(PointVec* vec, Point p);
int getVecItem(Vec* vec, size_t index);
Point getPointVecItem(PointVec* vec, size_t index);
bool containsPoint(PointVec* vec, Point p);
void destructVec(Vec* vec);
void destructPointVec(PointVec* vec);

#ifdef VEC_H_IMPLEMENTATION
// Implementations


Vec* createVec() {
  Vec* vec = (Vec*)malloc(sizeof(Vec));
  vec->size = 0;
  vec->capacity = 16;
  vec->items = (int*)calloc(vec->capacity, sizeof(int));
  return vec;
}

PointVec* createPointVec() {
  PointVec* vec = (PointVec*)malloc(sizeof(PointVec));
  vec->size = 0;
  vec->capacity = 16;
  vec->points = (Point*)calloc(vec->capacity, sizeof(Point));
  return vec;
}


void pushVecItem(Vec* vec, int item) {

  if (vec->size >= vec->capacity) {
    vec->capacity *= 2;
    vec->items = realloc(vec->items, vec->capacity * sizeof(int));
  }

  vec->items[vec->size++] = item;
}

void pushPointVecItem(PointVec* vec, Point p) {

  if (vec->size >= vec->capacity) {
    vec->capacity *= 2;
    vec->points = realloc(vec->points, vec->capacity * sizeof(Point));
  }

  vec->points[vec->size++] = p;
}

bool containsPoint(PointVec* vec, Point p) {
  size_t i;

  for (i = 0; i < vec->size; i++) {
    Point currPoint = vec->points[i];
    if (currPoint.x == p.x && currPoint.y == p.y) return true;
  }

  return false;
}

int getVecItem(Vec* vec, size_t index) {
  
  if (index < 0 || index > vec->size) {
    fprintf(stderr, "Vec: Please access item within the valid range");
    return 0.0f;
  }

  return vec->items[index];
}

Point getPointVecItem(PointVec* vec, size_t index) {

  if (index < 0 || index > vec->size) {
    fprintf(stderr, "PointVec: Please access item within the valid range");
    return (Point){.x = 0, .y = 0};
  }

  return vec->points[index];
}

void destructVec(Vec* vec) {
  free(vec->items);
  free(vec);
}

void destructPointVec(PointVec* vec) {
  free(vec->points);
  free(vec);
}


#endif // VEC_H_IMPLEMENTATION

#endif // VEC_H
