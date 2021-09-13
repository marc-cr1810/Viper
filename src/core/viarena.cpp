#include "viarena.h"

#include "vimem.h"
#include "error.h"
#include "../objects/listobject.h"

/* A simple arena block structure.

   Measurements with standard library modules suggest the average
   allocation is about 20 bytes and that most compiles use a single
   block.

   TODO: Think about a realloc API, maybe just for the last
   allocation?
*/

#define DEFAULT_BLOCK_SIZE	8192
#define ALIGNMENT			8

typedef struct _block
{
    /* Total number of bytes owned by this block available to pass out.
     * Read-only after initialization.  The first such byte starts at
     * ab_mem.
     */
    size_t ab_size;

    /* Total number of bytes already passed out.  The next byte available
     * to pass out starts at ab_mem + ab_offset.
     */
    size_t ab_offset;

    /* An arena maintains a singly-linked, NULL-terminated list of
     * all blocks owned by the arena.  These are linked via the
     * ab_next member.
     */
    struct _block *ab_next;

    /* Pointer to the first allocatable byte owned by this block.  Read-
     * only after initialization.
     */
    void *ab_mem;
} block;

/* The arena manages two kinds of memory, blocks of raw memory
   and a list of ViObject* pointers.  ViObjects are decrefed
   when the arena is freed.
*/

typedef struct _arena
{
    /* Pointer to the first block allocated for the arena, never NULL.
       It is used only to find the first block when the arena is
       being freed.
     */
    block *a_head;

    /* Pointer to the block currently used for allocation.  Its
       ab_next field should be NULL.  If it is not-null after a
       call to block_alloc(), it means a new block has been allocated
       and a_cur should be reset to point it.
     */
    block *a_cur;

    /* A Viper list object containing references to all the ViObject
       pointers associated with this arena.  They will be DECREFed
       when the arena is freed.
    */
    ViObject *a_objects;

#ifdef Vi_DEBUG
    /* Debug output */
    size_t total_allocs;
    size_t total_size;
    size_t total_blocks;
    size_t total_block_size;
    size_t total_big_blocks;
#endif
};

static block *block_new(size_t size)
{
    /* Allocate header and block as one unit.
       ab_mem points just past header. */
    block *b = (block *)Mem_Alloc(sizeof(block) + size);
    if (!b)
        return NULL;
    b->ab_size = size;
    b->ab_mem = (void *)(b + 1);
    b->ab_next = NULL;
    b->ab_offset = (char *)Vi_ALIGN_UP(b->ab_mem, ALIGNMENT) - (char *)(b->ab_mem);
    return b;
}

static void block_free(block *b)
{
    while (b)
    {
        block *next = b->ab_next;
        Mem_Free(b);
        b = next;
    }
}

static void *block_alloc(block *b, size_t size)
{
    void *p;
    assert(b);
    size = Vi_SIZE_ROUND_UP(size, ALIGNMENT);
    if (b->ab_offset + size > b->ab_size)
    {
        /* If we need to allocate more memory than will fit in
           the default block, allocate a one-off block that is
           exactly the right size. */
        /* TODO:Think about space waste at end of block */
        block *newbl = block_new(
            size < DEFAULT_BLOCK_SIZE ?
            DEFAULT_BLOCK_SIZE : size);
        if (!newbl)
            return NULL;
        assert(!b->ab_next);
        b->ab_next = newbl;
        b = newbl;
    }

    assert(b->ab_offset + size <= b->ab_size);
    p = (void *)(((char *)b->ab_mem) + b->ab_offset);
    b->ab_offset += size;
    return p;
}

ViArena *ViArena_New()
{
    ViArena *arena = (ViArena *)Mem_Alloc(sizeof(ViArena));
    if (!arena)
    {
        ViError_NoMemory();
        return NULL;
    }

    arena->a_head = block_new(DEFAULT_BLOCK_SIZE);
    arena->a_cur = arena->a_head;
    if (!arena->a_head)
    {
        Mem_Free((void *)arena);
        ViError_NoMemory();
        return NULL;
    }
    arena->a_objects = ViListObject_New(0);
    if (!arena->a_objects)
    {
        block_free(arena->a_head);
        Mem_Free((void *)arena);
        ViError_NoMemory();
        return NULL;
    }
#ifdef Vi_DEBUG
    arena->total_allocs = 0;
    arena->total_size = 0;
    arena->total_blocks = 1;
    arena->total_block_size = DEFAULT_BLOCK_SIZE;
    arena->total_big_blocks = 0;
#endif
    return arena;
}

void ViArena_Free(ViArena *arena)
{
    assert(arena);
#ifdef Vi_DEBUG
    fprintf(stderr,
            "alloc=%zu size=%zu blocks=%zu block_size=%zu big=%zu objects=%zu\n",
            arena->total_allocs, arena->total_size, arena->total_blocks,
            arena->total_block_size, arena->total_big_blocks,
            ViList_GET_SIZE(arena->a_objects));
#endif
    block_free(arena->a_head);
    /* This property normally holds, except when the code being compiled
       is sys.getobjects(0), in which case there will be two references.
    assert(arena->a_objects->ob_refcnt == 1);
    */

    ViObject_DECREF(arena->a_objects);
    Mem_Free(arena);
}

void *ViArena_Alloc(ViArena *arena, size_t size)
{
    void *p = block_alloc(arena->a_cur, size);
    if (!p)
    {
        ViError_NoMemory();
        return NULL;
    }
#ifdef Vi_DEBUG
    arena->total_allocs++;
    arena->total_size += size;
#endif
    /* Reset cur if we allocated a new block. */
    if (arena->a_cur->ab_next)
    {
        arena->a_cur = arena->a_cur->ab_next;
#ifdef Vi_DEBUG
        arena->total_blocks++;
        arena->total_block_size += arena->a_cur->ab_size;
        if (arena->a_cur->ab_size > DEFAULT_BLOCK_SIZE)
            ++arena->total_big_blocks;
#endif
    }
    return p;
}

int ViArena_AddViObject(ViArena *arena, ViObject *obj)
{
    int result = ViList_Append(arena->a_objects, obj);
    if (result >= 0)
        ViObject_DECREF(obj);
    return result;
}
