#define NOB_IMPLEMENTATION
#include "nob.h"

typedef enum{
  AF_UNKNOWN = 0,
  AF_EOF,
  AF_SQUARE_OPEN,
  AF_SQUARE_CLOSE,
  AF_ROUND_OPEN,
  AF_ROUND_CLOSE,
  AF_CURLY_OPEN,
  AF_CURLY_CLOSE,
  AF_ANGLE_OPEN,
  AF_ANGLE_CLOSE,
  AF_WHITESPACE
} AF_TokenKind;

const char* AF_TokenKind_str(AF_TokenKind kind){
  switch(kind){
    case AF_UNKNOWN: return "AF_UNKNOWN";
    case AF_EOF: return "AF_EOF";
    case AF_SQUARE_OPEN: return "AF_SQUARE_OPEN";
    case AF_SQUARE_CLOSE: return "AF_SQUARE_CLOSE";
    case AF_ROUND_OPEN: return "AF_ROUND_OPEN";
    case AF_ROUND_CLOSE: return "AF_ROUND_CLOSE";
    case AF_CURLY_OPEN: return "AF_CURLY_OPEN";
    case AF_CURLY_CLOSE: return "AF_CURLY_CLOSE";
    case AF_ANGLE_OPEN: return "AF_ANGLE_OPEN";
    case AF_ANGLE_CLOSE: return "AF_ANGLE_CLOSE";
    case AF_WHITESPACE: return "AF_WHITESPACE";
  }
  return "<invalid AF_TokenKind>";
}

typedef struct{
  AF_TokenKind kind;
  char* start;
  char* end;
} AF_Token;

typedef struct{
  char* start;
  char* curr;
  char* end;
  AF_Token token;
} AF_Lexer;

void af_lexer_init(AF_Lexer* self, char* str, size_t len){
  self->start = str;
  self->curr = str;
  self->end = str+len;
}

AF_TokenKind af_lexer_id_token(AF_Lexer* self){
  switch(*self->curr){
    case '[': return AF_SQUARE_OPEN;
    case ']': return AF_SQUARE_CLOSE;
    case '(': return AF_ROUND_OPEN;
    case ')': return AF_ROUND_CLOSE;
    case '{': return AF_CURLY_OPEN;
    case '}': return AF_CURLY_CLOSE;
    case '<': return AF_ANGLE_OPEN;
    case '>': return AF_ANGLE_CLOSE;
    default: return AF_UNKNOWN;
  }
}

bool af_lexer_is_whitespace(char c){
  return (c == ' '
    || c == '\t'
    || c == '\n'
    || c == '\r');
}

void af_lexer_consume_whitespace(AF_Lexer* self){
  while(self->curr < self->end
      && af_lexer_is_whitespace(*self->curr)
  ){
    self->curr++;
  }
}

AF_Token af_lexer_next(AF_Lexer* self){
  AF_Token token = {0};
  if(self->curr == NULL){
    self->curr = self->start;
  }


  token.kind = af_lexer_id_token(self);
  token.start = self->curr;

  if(token.kind != AF_UNKNOWN){
    self->curr++;
  }else if(af_lexer_is_whitespace(*self->curr)){
    // this feels sort of inconsistent but I cant be arsed to create a proper C + 
    //    C preprocessor lexer/parser as to prevent consumption of whitespace in 
    //    the right areas
    token.kind = AF_WHITESPACE;
    while(self->curr < self->end
        && af_lexer_is_whitespace(*self->curr)
    ){
      self->curr++;
    }
  }else if(self->curr < self->end){
    while(self->curr < self->end
        && af_lexer_id_token(self) == AF_UNKNOWN
    ){
      self->curr++;
    }
  }else{
    token.kind = AF_EOF;
  }

  token.end = self->curr;
  self->token = token;

  nob_log(NOB_INFO, "next token %s '%.*s'", AF_TokenKind_str(token.kind), (int)(token.end-token.start), token.start);

  return token;
}

bool af_lexer_next_is(AF_Lexer* self, AF_TokenKind kind){
  AF_Lexer snapshot = *self;
  
  AF_Token token = af_lexer_next(self);
  if(token.kind != kind){
    *self = snapshot;
    return false;
  }
  return true;
}

bool preprocess_content(
    Nob_String_Builder* out_head, 
    Nob_String_Builder* out_body, 
    Nob_String_Builder* out_tail, 
    Nob_String_Builder in, 
    const char* file_id_name,
    int* id_counter
){
  AF_Lexer lexer = {0};
  af_lexer_init(&lexer, in.items, in.count);

  while(lexer.token.kind != AF_EOF){

    // use lexer next to ensure token is consumed
    AF_Token token = af_lexer_next(&lexer);
    char* start = token.start;
    if(token.kind != AF_SQUARE_OPEN){
      nob_sb_append_buf(out_body, start, token.end-start);
      continue;
    }

    nob_log(NOB_INFO, "possible start of anon func: '%.*s'", 16, token.start);

    // note the careful usage of whitespace consumption
    // in most areas we want to leave it where it is
    if(af_lexer_next_is(&lexer, AF_WHITESPACE)){}; // consume
    if(af_lexer_next_is(&lexer, AF_SQUARE_CLOSE) == false){

      // check if the user is attempting to treat anon func as a C++ lambda
      AF_Token token = lexer.token;
      while(token.kind != AF_SQUARE_CLOSE){
        token = af_lexer_next(&lexer);
        if(token.kind == AF_EOF){
          nob_log(NOB_ERROR, "unexpected end of input");
          continue;
        }
      }
      if(af_lexer_next_is(&lexer, AF_WHITESPACE)){}; // consume
      if(af_lexer_next_is(&lexer, AF_ANGLE_OPEN)){
        nob_log(NOB_WARNING, "arg found in square brace, not anon func: '%.*s...'\n"
            "If you're attempting to create a lambda that captures variables: this is not supported\n"
            , 16, start);
      }
      nob_log(NOB_INFO, "not valid anon func syntax: %.*s", 32, start);
      nob_sb_append_buf(out_body, start, lexer.token.end-start);
      continue;
    }


    if(af_lexer_next_is(&lexer, AF_WHITESPACE)){}; // consume
    if(af_lexer_next_is(&lexer, AF_ANGLE_OPEN) == false){
      nob_log(NOB_INFO, "no angle brackets for return type, not anon func");
      nob_sb_append_buf(out_body, start, lexer.token.end-start);
      continue;
    }

    if(af_lexer_next_is(&lexer, AF_WHITESPACE)){}; // consume
    if(af_lexer_next_is(&lexer, AF_UNKNOWN) == false){
      nob_log(NOB_ERROR, "expected return type, got %.*s", (int)(lexer.token.end - lexer.token.start), lexer.token.start);
      continue;
    }

    AF_Token return_type = lexer.token;
    
    if(af_lexer_next_is(&lexer, AF_WHITESPACE)){}; // consume
    if(af_lexer_next_is(&lexer, AF_ANGLE_CLOSE) == false){
      nob_log(NOB_ERROR, "expected '>', got '%.*s'", 16, lexer.curr);
      continue;
    }
    
    if(af_lexer_next_is(&lexer, AF_WHITESPACE)){}; // consume
    if(af_lexer_next_is(&lexer, AF_ROUND_OPEN) == false) continue;
    nob_log(NOB_INFO, "start of arguments: '%.*s'", 16, lexer.token.start);

    AF_Token args = {0};
    if(af_lexer_next_is(&lexer, AF_WHITESPACE)){}; // consume
    if(af_lexer_next_is(&lexer, AF_UNKNOWN)){
      nob_log(NOB_INFO, "got arguments: '%.*s'", (int)(lexer.token.end - lexer.token.start), lexer.token.start);
      args = lexer.token;
      if(af_lexer_next_is(&lexer, AF_ROUND_CLOSE) == false){
        nob_log(NOB_ERROR, "expected ')' got '%s'", lexer.curr);
        continue;
      }
    }else if(af_lexer_next_is(&lexer, AF_ROUND_CLOSE)){
      nob_log(NOB_INFO, "anon func has no arguments");
    }else{
      AF_Token token = af_lexer_next(&lexer);
      nob_log(NOB_ERROR, "expect function args, got '%.*s'", (int)(token.end-token.start), token.start);
      continue;
    }

    if(af_lexer_next_is(&lexer, AF_WHITESPACE)){}; // consume
    if(af_lexer_next_is(&lexer, AF_CURLY_OPEN) == false){
      nob_log(NOB_ERROR, "expected '{' got '%s'", lexer.curr);
      continue;
    }

    int curly_depth = 1;

    AF_Token func_body = {0};
    func_body.start = lexer.token.end;

    while(curly_depth > 0){
      AF_Token token = af_lexer_next(&lexer);
      switch(token.kind){
        case AF_CURLY_OPEN: curly_depth++; break;
        case AF_CURLY_CLOSE: curly_depth--; break;
        case AF_EOF:
          nob_log(NOB_ERROR, "unexpecte end of file");
          return false;
        default: break;
      }
      if(curly_depth > 0){
        func_body.end = token.end;
      }
    }
    nob_log(NOB_INFO, "acquired anon func %d body: %.*s", *id_counter,(int)(func_body.end-func_body.start), func_body.start);
    
    // add prototype to head
    nob_sb_appendf(out_head, "%.*s afpp__%s_anon_func_%d(%.*s);\n", 
        (int)(return_type.end-return_type.start), return_type.start,
        file_id_name,
        *id_counter, 
        (int)(args.end-args.start), args.start);

    // add call to body
    nob_sb_appendf(out_body, "afpp__%s_anon_func_%d", file_id_name, *id_counter);

    // add implementation to tail
    nob_sb_appendf(out_tail, "%.*s afpp__%s_anon_func_%d(%.*s){%.*s}\n", 
        (int)(return_type.end-return_type.start), return_type.start,
        file_id_name,
        *id_counter, 
        (int)(args.end-args.start), args.start,
        (int)(func_body.end-func_body.start), func_body.start);
    (*id_counter)++;
  }

  if(out_head->count > 0){
    // recursively check tail for nested anon funcs
    Nob_String_Builder nested_head = {0};
    Nob_String_Builder nested_body = {0};
    Nob_String_Builder nested_tail = {0};
    if(preprocess_content(&nested_head, &nested_body, &nested_tail, *out_tail, file_id_name, id_counter) == false){
      if(nested_head.items != NULL) nob_sb_free(nested_head);
      if(nested_body.items != NULL) nob_sb_free(nested_body);
      if(nested_tail.items != NULL) nob_sb_free(nested_tail);
      return false;
    }

    // overwrite tail with preprocessed content
    out_tail->count = 0;
    nob_sb_append_buf(out_tail, nested_head.items, nested_head.count);
    nob_sb_append_buf(out_tail, nested_body.items, nested_body.count);
    nob_sb_append_buf(out_tail, nested_tail.items, nested_tail.count);

    if(nested_head.items != NULL) nob_sb_free(nested_head);
    if(nested_body.items != NULL) nob_sb_free(nested_body);
    if(nested_tail.items != NULL) nob_sb_free(nested_tail);
  }

  return true;
}

// returns path to preprocessed file
const char* preprocess_file(const char* file, const char* output_dir){
  const char* result = NULL;

  char* file_id_name = nob_temp_strdup(file);
  for(char* s = file_id_name; *s != '\0'; ++s){
    // replace "weird" chars with underscore '_'
    if((*s >= 'a' && *s <= 'z') == false
        && (*s >= 'A' && *s <= 'Z') == false
    ){
      *s = '_';
    }
  }

  Nob_String_Builder in = {0};

  if(nob_read_entire_file(file, &in) == false) return NULL;

  Nob_String_Builder out_head = {0};
  Nob_String_Builder out_body = {0};
  Nob_String_Builder out_tail = {0};
  
  int id_counter = 0;
  if(preprocess_content(&out_head, &out_body, &out_tail, in, file_id_name, &id_counter) == false){
    goto RET;
  }

  Nob_String_Builder out = {0};
  nob_sb_append_cstr(&out, "// +++ AFPP HEAD +++\n");
  nob_sb_append_buf(&out, out_head.items, out_head.count);
  nob_sb_append_cstr(&out, "\n// --- AFPP HEAD ---\n");
  nob_sb_append_buf(&out, out_body.items, out_body.count);
  nob_sb_append_cstr(&out, "// +++ AFPP TAIL +++\n");
  nob_sb_append_buf(&out, out_tail.items, out_tail.count);
  nob_sb_append_cstr(&out, "\n// --- AFPP TAIL ---\n");
  result = nob_temp_sprintf("%s/%s.afpp.c", output_dir, file);
  if(nob_write_entire_file(result, out.items, out.count) == false){
    result = NULL;
  }

RET:
  if(in.items != NULL) nob_sb_free(in);
  if(out.items != NULL) nob_sb_free(out);
  if(out_head.items != NULL) nob_sb_free(out_head);
  if(out_body.items != NULL) nob_sb_free(out_body);
  return result;
}

void usage(const char* program){
  printf("%s [options] <file> [<file> ...]\n", program);
  printf("OPTIONS:\n");
  printf("  -h              : show this help\n");
  printf("  -v              : verbose logging\n");
  printf("  -d <directory>  : set output directory\n");
}

int main(int argc, char** argv){
  nob_minimal_log_level = NOB_WARNING;

  const char* output_dir = "./";
  for(int i = 1; i < argc; ++i){
    if(argv[i][0] == '-'){
      switch(argv[i][1]){
        case 'h': usage(argv[0]); break;
        case 'v': nob_minimal_log_level = NOB_INFO; break;
        case 'd': 
          if(i+1 < argc){
            output_dir = argv[i+1];
            nob_mkdir_if_not_exists(output_dir);
            ++i;
          }else{
            printf("ERROR: -d missing argument: <directory>\n");
            usage(argv[0]);
            return 1;
          }
          break;
        default:{
          printf("ERROR: unknown option: '%s'\n", argv[i]);
          usage(argv[0]);
          return 1;
        }break;
      }
    }else{
      const char* res = preprocess_file(argv[i], output_dir);
      if(res == NULL){
        nob_log(NOB_ERROR, "failed to preprocess: %s",argv[i]);
        return 1;
      }else{
        nob_log(NOB_INFO, "succesfully preprocessed: %s", argv[i]);
      }
      nob_temp_reset();
    }
  }
}
