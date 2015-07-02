typedef struct __header_t  
{
    int size;
    int magic;
} header_t;

typedef struct __node_t
{
    int size;
    int magic;
    struct __node_t *next;
} node_t;


char * start_of_memory;
char * end_of_memory;
node_t *head = mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
head->size = 4084;
head->magic = 98765;
head->next = NULL;
start_of_memory = (char *)head;
end_of_memory = (char *)head + 4096;


void *malloc(size_t size)
{
    node_t *cell = head;
    while(1)
    {   
        if(cell->size >= size)
        {
            if(cell->size <= size)//if it is an exact fit just return it
            {
                header_t *new_cell = (header_t *)cell;
                new_cell->magic=1234567;
                new_cell->size=size;

                audit();
                return (void *)new_cell + sizeof(header_t);
            }
            //otherwise lets carve up this cell
            int new_size = cell->size - size - sizeof(node_t);

            header_t *new_cell = (header_t *)cell;

            cell = (void *)cell + sizeof(node_t) + size;
            cell->size = new_size;
            cell->magic = 98765;
            new_cell->magic = 1234567;
            new_cell->size = size;

            head = cell;//set head so I can point to it in the next freed cell
            audit();
            return (void *)new_cell + sizeof(header_t);
        }
        else if(cell->next != NULL)
        {
            cell = cell->next;
        }
        else
        {
            printf("ran out of size, need to sbrk\n");
            cell = sbrk(4096);
            cell->size=4084;
            cell->magic=98765;
            cell->next=NULL;
            break;
        }
    }

}



void free(void *ptr)
{

    header_t *hptr = (void *)ptr - sizeof(header_t);
    assert(hptr->magic == 1234567);
    node_t *cell = (node_t *)hptr;
    node_t *next_cell = head;
    node_t *before_cell;

    cell->size = hptr->size;
    cell->magic = 98765;
    if(cell < head)//must be the first one on the list
    {

        if((void *)cell + sizeof(node_t) + cell->size == head)
        {

            cell->size=cell->size + head->size + sizeof(node_t);
            cell->next=head->next;
            head = cell;
        }
        else
        {
            cell->next = next_cell;
        }
    }
    else
    {
        while(1)
        {
            if(next_cell->next != NULL)
            {
                before_cell = next_cell;
                next_cell = next_cell->next;
            }
            else
            {
                printf("We're gonna have a problem here\n");
            }
            if(cell < next_cell)
            {
                if((void *)cell + sizeof(node_t) + cell->size == next_cell)
                {
                    cell->size=cell->size + next_cell->size + sizeof(node_t);
                    cell->next=next_cell->next;
                }
                if((void *)cell - sizeof(node_t) - before_cell->size == before_cell)
                {
                    before_cell->size=cell->size + before_cell->size + sizeof(node_t);
                    before_cell->next=cell->next;
                }
                else
                {
                    before_cell->next = cell;
                    cell->next = next_cell;
                }
                break;
            }

        }

    }
    if(cell < head)
    {
        head = cell;
    }
}

