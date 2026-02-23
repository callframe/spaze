#include "spaze/list.h"
#include "spaze/common.h"
#include <stddef.h>

void list_insert_after(struct list_s *list, struct link_s *link,
                       struct link_s *new_link) {
  assert_notnull(list);
  assert_notnull(link);
  assert_notnull(new_link);

  new_link->next = link->next;
  new_link->prev = link;

  if (link->next)
    link->next->prev = new_link;
  else
    list->tail = new_link;

  link->next = new_link;
}

void list_append(struct list_s *list, struct link_s *new_link) {
  assert_notnull(list);
  assert_notnull(new_link);

  if (list->tail == NULL) {
    list->head = new_link;
    list->tail = new_link;
    return;
  }

  list_insert_after(list, list->tail, new_link);
}

void list_remove(struct list_s *list, struct link_s *link) {
  assert_notnull(list);
  assert_notnull(link);

  if (link->prev)
    link->prev->next = link->next;
  else
    list->head = link->next;

  if (link->next)
    link->next->prev = link->prev;
  else
    list->tail = link->prev;
}
