
main()
{
  int i,j;
  for(i = 0 ; i < 8 ; i++) {
    for(j = 0; j < 8 ; j++) {
      printf("%d,",((i<<3)+j)<<16);
    }
    putchar('\n');
  }
}
