struct m_list {
   void *address;
   unsigned int size;
   unsigned int in_use:1;
   m_list *next;
   m_list( void *_address, long _size) { size = _size; address = _address; next = NULL; in_use = 0; }
};

//1 KB buffer
char buffer[1024];



unsigned int get_next_two_power( unsigned int n ) {
   unsigned int num = 1;
   while( num < n ) {
       num <<= 1;
   }
   return num;
}




void * malloc( unsigned int size, m_list **memlist ) {
   if ( NULL == memlist || NULL == *memlist ) return NULL;
   int alloc_size = get_next_two_power( size );

   m_list *p = *memlist;
   while( p ) {
       if( p->in_use == 0 ) {
           if( alloc_size == p->size ) {
               p->in_use = 1;
               return p->address;
           }
           if ( alloc_size < p->size ) {
               m_list *node = new m_list(NULL, p->size >> 1);
               node->address = (void *)((int)(p->address) + (p->size >> 1));
               node->next = p->next;
               p->next = node;
               p->size = p->size >> 1;
               return malloc( alloc_size, memlist );
           }
       }
       p = p->next;
   }

   return NULL;
}



void merge_buddies( m_list **memlist ) {
   m_list *curr = *memlist, *next = curr;
   bool is_merge = false;
   while( curr ) {
       if( curr->in_use == 0 ) {
           if( NULL != curr->next ) {
               next = curr->next;
               if( 0 == next->in_use && (next->size == curr->size) ) {
                   curr->size += next->size;
                   curr->next = next->next;
                   is_merge = true;
                   free(next);
               }
           }
       }
       curr = curr->next;
   }
   if ( is_merge ) {
       merge_buddies( memlist );
   }
}




void free( void * address, m_list **memlist ) {
   if( NULL == address || NULL == memlist || NULL == *memlist ) return;
   m_list *curr = *memlist, *back, *next;

   while( curr ) {
       if( curr->address == address ) {
           curr->in_use = 0;
           merge_buddies( memlist );
           return;
       }
       curr = curr->next;
   }
}




void * realloc( void * address, unsigned int size, m_list **memlist ) {
 free(address, memlist);
 void *adr = malloc(size, memlist);
 memcpy(adr, address, size );

 return adr;
}


int main() {
 m_list *memlist = new m_list((void *)buffer, 1024);
 printf("buffer address:: %p\n", buffer);
 printf("memlist address:: %p\n", memlist->address);

 int *p1 = (int *)malloc(sizeof(int)*100, &memlist);
 for( int i = 0; i < 100; i++ ) {
  p1[i] = i;
 }

 p1 = (int *)realloc(p1, sizeof(int)*256, &memlist);
 for( int i = 0; i < 100; i++ ) {
  cout << p1[i] << " * ";
 }
 cout << endl;

 free( p1, &memlist );

 char *p2 = (char *)malloc(sizeof(char)*240, &memlist);
 *p2 = '\0';
 strcpy( p2, "Mustafa Veysi Soyvural\0" );
 cout << p2 << endl;
 free( p2, &memlist );

 return 0;
}

void* calloc(size_t num, size_t len) {
  void* ptr = malloc(num * len);

  /* Set the allocated array to 0's.*/
  if (ptr != NULL) {
    memset(ptr, 0, num * len);
  }

  return ptr;
}

