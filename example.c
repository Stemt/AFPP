#include <stdio.h>

typedef int(*Callback)(int a, int b);

int main(void){
  
  // basic sub callback
  Callback sub = [ ] <int> (int a, int b){
    return a - b;
  };

  printf("sub: %d\n", sub(1,2));

  // multi nested callback
  Callback add = []<int>(int a, int b){
    Callback nested_add = []<int>(int a, int b){
      Callback nested_nested_add = []<int>(int a, int b){
        return a + b;
      };
      return nested_nested_add(a,b);
    };
    return nested_add(a,b);
  };
  printf("add: %d\n", add(1,2));

  // inline call
  printf("inline: %d\n", []<int>(){ return 2; }());
  return 0;
}
