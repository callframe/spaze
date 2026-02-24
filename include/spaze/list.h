#pragma once

struct link_s {
  struct link_s *next;
  struct link_s *prev;
};

#define link_init(next, prev) ((struct link_s){.next = (next), .prev = (prev)})

struct list_s {
  struct link_s *head;
  struct link_s *tail;
};

void list_insert_after(struct list_s *list, struct link_s *link,
                       struct link_s *new_link);
void list_push(struct list_s *list, struct link_s *link);
struct link_s *list_pop_front(struct list_s *list);
void list_remove(struct list_s *list, struct link_s *link);

#define list_for_each_reversed(list, link)                                     \
  for (struct link_s *link = (list)->tail,                                     \
                     *link##_next_ = link ? link->prev : NULL;                 \
       link != NULL;                                                           \
       link = link##_next_, link##_next_ = link ? link->prev : NULL)
#define list_for_each(list, link)                                              \
  for (struct link_s *link = (list)->head,                                     \
                     *link##_next_ = link ? link->next : NULL;                 \
       link != NULL;                                                           \
       link = link##_next_, link##_next_ = link ? link->next : NULL)
