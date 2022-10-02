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

#define dll_init_elem(elem, name) \
  do {                            \
    (elem)->name.prev = NULL;     \
    (elem)->name.next = NULL;     \
  } while (0);

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

#define dll_add_after(list, elem, name, previous) \
  do {                                            \
    (elem)->name.next = (previous)->name.next;    \
    (elem)->name.prev = (previous);               \
    (previous)->name.next = (elem);               \
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

#define dll_remove(list, elem, name)                    \
  do {                                                  \
    if ((elem)->name.prev != NULL) {                    \
      (elem)->name.prev->name.next = (elem)->name.next; \
    }                                                   \
    if ((elem)->name.next != NULL) {                    \
      (elem)->name.next->name.prev = (elem)->name.prev; \
    }                                                   \
    if ((list)->head == (elem)) {                       \
      (list)->head = (elem)->name.next;                 \
    }                                                   \
    if ((list)->tail == (elem)) {                       \
      (list)->tail = (elem)->name.prev;                 \
    }                                                   \
  } while (0);

#endif
