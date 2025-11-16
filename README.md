# AFPP

# Simple Example

Using AFPP one can define an anonymous function like so.

```C
#include <stdio.h>

typedef int(*Callback)(int a, int b);

int main(void){
  
  Callback sub = []<int>(int a, int b){
    return a - b;
  };

  printf("sub: %d\n", sub(1,2));
  return 0;
}
```

Then to compile this example you first have to pass the file through AFPP and then compile the output file named like `<og name>.afpp.c`.

```sh
afpp example.c && cc -o example example.c.afpp.c
```

Essentially what AFPP does is it scans for the pattern `[]<return type>(args){ function body }` and seperates it into a prototype, call and implementation.
All generated prototypes are appended at the beginning or 'head' of the file, calls are replaced inline in the 'body' of the file and implementations are appended at the end or 'tail' of the file.

The resulting output looks like this:

```C
// +++ AFPP HEAD +++
int afpp__example_c_anon_func_0(int a, int b);

// --- AFPP HEAD ---
#include <stdio.h>

typedef int(*Callback)(int a, int b);

int main(void){
  
  Callback sub = afpp__example_c_anon_func_0;

  printf("sub: %d\n", sub(1,2));

  return 0;
}
// +++ AFPP TAIL +++
int afpp__example_c_anon_func_0(int a, int b){
    return a - b;
  }

// --- AFPP TAIL ---

```
