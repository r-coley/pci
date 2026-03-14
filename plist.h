/*
 * plist.h
 */
#ifndef PLIST_H
#define PLIST_H

#define INLINE

typedef struct list_head {
	struct list_head *next, *prev;
} List_t;

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(X) List_t X = LIST_HEAD_INIT(X)

#define INIT_LIST_HEAD(ptr) \
	do { (ptr)->next = (ptr); (ptr)->prev = (ptr); } while(0)

#define list_entry(Ptr,Type,Elem) \
	((Type *)((char *)(Ptr) - offsetof(Type,Elem)))

#define list_for_each(pos,head) \
	for(pos=(List_t *)(head)->next; pos != (head); pos=pos->next)	

static INLINE void __list_add(List_t *new,List_t *prev,List_t *next)
{
	next->prev=new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

static INLINE void list_add(List_t *new,List_t *head)
{
	__list_add(new,head,head->next);
}

static INLINE void list_add_tail(List_t *new, List_t *head)
{
	__list_add(new, head->prev, head);
}

static INLINE void __list_del(List_t *prev,List_t *next)
{
	next->prev=prev;
	prev->next=next;
}

static INLINE void
list_del(List_t *entry)
{
	__list_del(entry->prev, entry->next);
}

static INLINE int list_empty(List_t *head)
{
	return head->next == head;
}

static INLINE void list_splice(List_t *list, List_t *head)
{
	List_t *first=list->next;

	if (first != list) {
		List_t *last = list->prev;
		List_t *at = head->next;

		first->prev = head;
		head->next = first;

		last->next = at;
		at->prev = last;
	}
}

#ifdef MAIN
#include <stdio.h>
#include <sys/types.h>

#define MAX_NAME_LEN	32
#define MAX_ID_LEN	10

LIST_HEAD(myhead);

#define I2C_TYPE	1
#define SPI_TYPE	2

char *dev_name[] = {
	"none",	
	"I2C",	
	"SPI"
};

struct mylist
{
	int 	type;
	char 	name[MAX_NAME_LEN];
	List_t 	list;
};

void
display_list(List_t *list_head)
{
	int	i=0;
	List_t *p;
	struct mylist *entry;

	printf("-------------list-----------\n");
	list_for_each(p,&myhead)
	{
		printf("Node[%d]\n",++i);
		entry=list_entry(p,struct mylist,list);
		printf("\ttype: %s\n",dev_name[entry->type]);
		printf("\tname: %s\n",entry->name);
	}
	printf("-------------end-----------\n");
}

main()
{
	struct mylist node1, node2, *nptr;

	INIT_LIST_HEAD(&myhead);

	nptr=(struct mylist *)calloc(1,sizeof(struct mylist));
	nptr->type = SPI_TYPE;
	strcpy(nptr->name,"abcdefhij");

	node1.type = I2C_TYPE;
	strcpy(node1.name,"1yikoulinux");

	node2.type = I2C_TYPE;
	strcpy(node2.name,"2yikoupeng");

	list_add(&node1.list,&myhead);
	list_add(&node2.list,&myhead);
	list_add(&nptr->list,&myhead);

	display_list(&myhead);

	printf("Deleting Node1\n");
	list_del(&node1.list);

	display_list(&myhead);
}
#endif /*MAIN*/

#endif /*PLIST_H*/
