#ifndef LVL5_CONTEXT

#include "lvl5_types.h"
#include "lvl5_arena.h"

typedef enum {
  Alloc_Op_NONE,
  Alloc_Op_ALLOC,
  Alloc_Op_FREE,
  Alloc_Op_REALLOC,
  Alloc_Op_FREE_ALL,
} Alloc_Op;

#define ALLOCATOR(name) byte *(name)(Alloc_Op type, Mem_Size size, void *allocator_data, void *old_ptr, Mem_Size *old_size, Mem_Size align)
typedef ALLOCATOR(*Allocator);

typedef struct {
  Allocator allocator;
  void *allocator_data;
  
  Arena scratch;
} Context;

typedef struct {
  Context stack[32];
  i32 stack_count;
} Global_Context_Info;

globalvar thread_local Global_Context_Info *global_context_info = null;

Context *get_context() {
  Context *result = global_context_info->stack + global_context_info->stack_count - 1;
  return result;
}

void push_context(Context ctx) {
  global_context_info->stack[global_context_info->stack_count++] = ctx;
}

void pop_context() {
  global_context_info->stack_count--;
}

byte *alloc(Mem_Size size) {
  Context *ctx = get_context();
  byte *result = ctx->allocator(Alloc_Op_ALLOC, size, ctx->allocator_data, null, 0, 16);
  return result;
}

ALLOCATOR(arena_allocator) {
  Context *ctx = get_context();
  Arena *arena = (Arena *)ctx->allocator_data;
  
  byte *result = 0;
  switch (type) {
    case Alloc_Op_ALLOC: {
      result = _arena_push_memory(arena, size, align);
    } break;
    
    case Alloc_Op_FREE_ALL: {
      arena->size = 0;
    } break;
    
    invalid_default_case;
  }
  
  return result;
}

ALLOCATOR(scratch_allocator) {
  Context *ctx = get_context();
  byte *result = arena_allocator(type, size, &ctx->scratch, old_ptr, old_size, align);
  return result;
}

byte *scratch_alloc(Mem_Size size) {
  byte *result = scratch_allocator(Alloc_Op_ALLOC, size, null, null, 0, 16);
  return result;
}

void scratch_reset() {
  Context *ctx = get_context();
  ctx->scratch.size = 0;
}


#define LVL5_CONTEXT
#endif
