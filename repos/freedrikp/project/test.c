

int main(){
  int* n = malloc(sizeof(int));
  *n = 456;
  free(n);
}
