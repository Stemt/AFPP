# AFPP

## Simple Example

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

## Limitations

### These are NOT Lambdas

The syntax was purely chosen because the pattern is easy to identify in normal C code.

Capturing variables is supported.

### Text Only Processing

Because AFPP is a preprocessor purely working by searching and replacing text patterns the scope of its error reporting is limited.
Though I've noticed that the compiler often is able to give you enough information to fix a problem.

I've also not tested much how AFPP interacts with the normal C pre-processor, AFPP tries to leave everything apart from the anonoymous function syntax alone,
but since AFPP is applied before the normal preprocessor if they do interact the results are likely to be unexpected.

### Language Servers

Language servers won't understand what you're doing so if you're using AFPP, your IDE will likely report an error on every place an anonymous function is defined.
Though with `clangd` I've noticed that if you assign it to a function pointer variable you'll only get an error on the assignment and not in the other places of your code.

Trust the AFPP error/warnings and your compiler and everything should be good.

## FAQ

### Is this production ready?

Haha no, this was a hobby project slapped together in a weekend, use this at your own risk.

### Can you nest anonymous functions?

Hell yeah brother!
```C
  Callback add = []<int>(int a, int b){
    Callback nested_add = []<int>(int a, int b){
      Callback nested_nested_add = []<int>(int a, int b){
        return a + b;
      };
      return nested_nested_add(a,b);
    };
    return nested_add(a,b);
  };
```

### Can you fix the indentation of function implementations?

no (am lazy)
