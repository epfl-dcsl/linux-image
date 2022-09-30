#ifndef __INCLUDE_DLL_H__
#define __INCLUDE_DLL_H__

#define dll_elem(tpe, name) \
  struct {                  \
    tpe* prev;              \
    tpe* next;              \
  } name;

#define dll_list(tpe, name) \
  struct {                  \
    tpe* head;              \
    tpe* tail;              \
  } name;

#define dll_init_list(list) \
  (list)->head = NULL;      \
  (list)->tail = NULL;

#define dll_foreach(list, curr, name) \
  for (curr = (list)->head; (curr) != NULL; (curr) = (curr)->name.next)

#define dll_add(list, elem, name)       \
  do {                                  \
    if ((list)->head == NULL) {         \
      (list)->head = (elem);            \
      (list)->tail = (elem);            \
    } else {                            \
      (list)->tail->name.next = (elem); \
      (elem)->name.prev = (list)->tail; \
      (list)->tail = (elem);            \
    }                                   \
  } while (0);

#define dll_add_after(list, elem, name, prev) \
  do {                                        \
    (elem)->name.next = (prev)->name.next;    \
    (elem)->name.prev = (prev);               \
    (prev)->name.next = (elem);               \
  } while (0);

#define dll_add_first(list, elem, name) \
  do {                                  \
    (elem)->name.prev = NULL;           \
    (elem)->name.next = (list)->head;   \
    (list)->head = (elem);              \
    if ((list)->tail == NULL) {         \
      (list)->tail = (elem);            \
    }                                   \
  } while (0);

#endif
