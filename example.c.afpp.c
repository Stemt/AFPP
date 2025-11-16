// +++ AFPP HEAD +++
int afpp__example_c_anon_func_0(int a, int b);
int afpp__example_c_anon_func_1(int a, int b);
int afpp__example_c_anon_func_2();

// --- AFPP HEAD ---
#include <stdio.h>

typedef int(*Callback)(int a, int b);

int main(void){
  
  // basic sub callback
  Callback sub = afpp__example_c_anon_func_0;

  printf("sub: %d\n", sub(1,2));

  // multi nested callback
  Callback add = afpp__example_c_anon_func_1;
  printf("add: %d\n", add(1,2));

  // inline call
  printf("inline: %d\n", afpp__example_c_anon_func_2());
  return 0;
}
// +++ AFPP TAIL +++
int afpp__example_c_anon_func_3(int a, int b);
int afpp__example_c_anon_func_0(int a, int b){
    return a - b;
  }
int afpp__example_c_anon_func_1(int a, int b){
    Callback nested_add = afpp__example_c_anon_func_3;
    return nested_add(a,b);
  }
int afpp__example_c_anon_func_2(){ return 2; }
int afpp__example_c_anon_func_4(int a, int b);
int afpp__example_c_anon_func_3(int a, int b){
      Callback nested_nested_add = afpp__example_c_anon_func_4;
      return nested_nested_add(a,b);
    }
int afpp__example_c_anon_func_4(int a, int b){
        return a + b;
      }

// --- AFPP TAIL ---
