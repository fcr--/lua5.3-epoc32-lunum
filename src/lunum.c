#include <_ansi.h>
#undef _STRICT_ANSI
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "lualib.h"
#include "lauxlib.h"
#include "lunum.h"

#ifdef _STRICT_ANSI
#error "strict ansi is defined"
#endif


#define LUA_NEW_METAMETHOD(luastate, obj, funcname) {           \
    lua_pushfstring((luastate), "__%s", (#funcname));           \
    lua_pushcfunction((luastate), (luaC_##obj##__##funcname));  \
    lua_settable((luastate), -3);                               \
  }

#define LUA_NEW_MODULEMETHOD(luastate, obj, funcname) {         \
    lua_pushfstring((luastate), "%s", (#funcname));             \
    lua_pushcfunction((luastate), (luaC_##obj##_##funcname));   \
    lua_settable((luastate), -3);                               \
  }

#define LUA_NEW_MODULEDATA(luastate, obj, dataname) {   \
    lua_pushstring((luastate), (#dataname));            \
    lua_pushnumber((luastate), (obj));                  \
    lua_settable((luastate), -3);                       \
  }



static int luaC_lunum_array(lua_State *L);
static int luaC_lunum_zeros(lua_State *L);
static int luaC_lunum_range(lua_State *L);
static int luaC_lunum_resize(lua_State *L);


static int luaC_lunum_sin(lua_State *L);
static int luaC_lunum_cos(lua_State *L);
static int luaC_lunum_tan(lua_State *L);

static int luaC_lunum_sin(lua_State *L);
static int luaC_lunum_cos(lua_State *L);
static int luaC_lunum_tan(lua_State *L);

static int luaC_lunum_asin(lua_State *L);
static int luaC_lunum_acos(lua_State *L);
static int luaC_lunum_atan(lua_State *L);

static int luaC_lunum_sinh(lua_State *L);
static int luaC_lunum_cosh(lua_State *L);
static int luaC_lunum_tanh(lua_State *L);

static int luaC_lunum_exp(lua_State *L);
static int luaC_lunum_log(lua_State *L);
static int luaC_lunum_log10(lua_State *L);

static int luaC_lunum_slice(lua_State *L);
static int luaC_lunum_loadtxt(lua_State *L);
static int luaC_lunum_fromfile(lua_State *L);



static int luaC_array__tostring(lua_State *L);
static int luaC_array__call(lua_State *L);
static int luaC_array__index(lua_State *L);
static int luaC_array__newindex(lua_State *L);
static int luaC_array__add(lua_State *L);
static int luaC_array__sub(lua_State *L);
static int luaC_array__mul(lua_State *L);
static int luaC_array__div(lua_State *L);
static int luaC_array__pow(lua_State *L);
static int luaC_array__unm(lua_State *L);
static int luaC_array__gc(lua_State *L);


static int   _array_binary_op1(lua_State *L, enum ArrayOperation op);
static int   _array_binary_op2(lua_State *L, enum ArrayOperation op);

static void _unary_func(lua_State *L, double(*f)(double), int cast);
static void _push_value(lua_State *L, enum ArrayType T, void *v);
static int _get_index(lua_State *L, struct Array *A);



// Functions used by unary predicates
// -----------------------------------------------------------------------------
static double runm(double x) { return -x; }   // unary minus, real
// -----------------------------------------------------------------------------





EXPORT_C int luaopen_lunum(lua_State *L)
{
  lua_settop(L, 0); // start with an empty stack

  // Create the 'array' metatable
  // ---------------------------------------------------------------------------
  luaL_newmetatable(L, "array");
  LUA_NEW_METAMETHOD(L, array, tostring);
  LUA_NEW_METAMETHOD(L, array, call);
  LUA_NEW_METAMETHOD(L, array, index);
  LUA_NEW_METAMETHOD(L, array, newindex);
  LUA_NEW_METAMETHOD(L, array, add);
  LUA_NEW_METAMETHOD(L, array, sub);
  LUA_NEW_METAMETHOD(L, array, mul);
  LUA_NEW_METAMETHOD(L, array, div);
  LUA_NEW_METAMETHOD(L, array, pow);
  LUA_NEW_METAMETHOD(L, array, unm);
  LUA_NEW_METAMETHOD(L, array, gc);
  lua_pop(L, 1);


  // Create the 'lunum' table
  // ---------------------------------------------------------------------------
  lua_newtable(L);
  LUA_NEW_MODULEMETHOD(L, lunum, array);
  LUA_NEW_MODULEMETHOD(L, lunum, zeros);
  LUA_NEW_MODULEMETHOD(L, lunum, range);
  LUA_NEW_MODULEMETHOD(L, lunum, resize);

  LUA_NEW_MODULEMETHOD(L, lunum, sin);
  LUA_NEW_MODULEMETHOD(L, lunum, cos);
  LUA_NEW_MODULEMETHOD(L, lunum, tan);

  LUA_NEW_MODULEMETHOD(L, lunum, asin);
  LUA_NEW_MODULEMETHOD(L, lunum, acos);
  LUA_NEW_MODULEMETHOD(L, lunum, atan);

  LUA_NEW_MODULEMETHOD(L, lunum, sinh);
  LUA_NEW_MODULEMETHOD(L, lunum, cosh);
  LUA_NEW_MODULEMETHOD(L, lunum, tanh);

  LUA_NEW_MODULEMETHOD(L, lunum, exp);
  LUA_NEW_MODULEMETHOD(L, lunum, log);
  LUA_NEW_MODULEMETHOD(L, lunum, log10);

  LUA_NEW_MODULEMETHOD(L, lunum, loadtxt);
  LUA_NEW_MODULEMETHOD(L, lunum, fromfile);

  LUA_NEW_MODULEMETHOD(L, lunum, slice);


  LUA_NEW_MODULEDATA(L, ARRAY_TYPE_BOOL   , bool);
  LUA_NEW_MODULEDATA(L, ARRAY_TYPE_CHAR   , char);
  LUA_NEW_MODULEDATA(L, ARRAY_TYPE_SHORT  , short);
  LUA_NEW_MODULEDATA(L, ARRAY_TYPE_INT    , int);
  LUA_NEW_MODULEDATA(L, ARRAY_TYPE_LONG   , long);
  LUA_NEW_MODULEDATA(L, ARRAY_TYPE_FLOAT  , float);
  LUA_NEW_MODULEDATA(L, ARRAY_TYPE_DOUBLE , double);

  lua_setglobal(L, "lunum");
#include "array_class.lc" // sets the lunum.__array_methods table

  lua_getglobal(L, "lunum");
  return 1;
}


// *****************************************************************************
// Implementation of lunum.array metatable
//
// *****************************************************************************
int luaC_array__gc(lua_State *L)
{
  struct Array *A = lunum_checkarray1(L, 1);
  array_del(A);
  return 0;
}

int luaC_array__tostring(lua_State *L)
{
  int n;
  struct Array *A = lunum_checkarray1(L, 1);

  lua_pushstring(L, "  [ ");
  int nstr = 1;
  for (n=0; n<A->size; ++n) {

    char s[64];

    switch (A->dtype) {
    case ARRAY_TYPE_BOOL    : sprintf(s, "%s" , ((Bool*)A->data)[n]?"true":"false"); break;
    case ARRAY_TYPE_CHAR    : sprintf(s, "%d" , ((char   *)A->data)[n]); break;
    case ARRAY_TYPE_SHORT   : sprintf(s, "%d" , ((short  *)A->data)[n]); break;
    case ARRAY_TYPE_INT     : sprintf(s, "%d" , ((int    *)A->data)[n]); break;
    case ARRAY_TYPE_LONG    : sprintf(s, "%ld", ((long   *)A->data)[n]); break;
    case ARRAY_TYPE_FLOAT   : sprintf(s, "%g" , ((float  *)A->data)[n]); break;
    case ARRAY_TYPE_DOUBLE  : sprintf(s, "%g" , ((double *)A->data)[n]); break;
    }

    if (n == A->size-1) {
      lua_pushfstring(L, "%s", s);
    }
    else {
      lua_pushfstring(L, "%s, ", s);
    }
    if ((n+1) % 10 == 0 && n != 0 && n != A->size-1) {
      lua_pushstring(L, "\n    "); ++nstr;
    }
  }
  lua_pushstring(L, " ]"); ++nstr;
  lua_concat(L, A->size + nstr);

  return 1;
}

int luaC_array__call(lua_State *L)
{
  struct Array *A = lunum_checkarray1(L, 1);
  int nind = lua_gettop(L) - 1;
  int d;

  if (nind != A->ndims) {
    luaL_error(L, "wrong number of indices (%d) for array of dimension %d",
               nind, A->ndims);
    return 0;
  }
  const int Nd = A->ndims;
  int *stride = (int*) malloc(A->ndims * sizeof(int));
  stride[Nd-1] = 1;

  for (d=Nd-2; d>=0; --d) {
    stride[d] = stride[d+1] * A->shape[d+1];
  }

  int m = 0;
  for (d=0; d<A->ndims; ++d) {
    int i = lua_tointeger(L, d+2);
    m += i*stride[d];
  }

  _push_value(L, A->dtype, (char*)A->data + m*array_sizeof(A->dtype));
  free(stride);

  return 1;
}

int luaC_array__index(lua_State *L)
{
  struct Array *A = lunum_checkarray1(L, 1);

  // Figure out what is the format of the input index. If it's a number or a
  // table of numbers, then pass it along to _get_index. If it's a table of
  // tables or numbers, then assume it's a slice. If it's an array of bools,
  // then use it as a mask.
  // ---------------------------------------------------------------------------

  if (lunum_hasmetatable(L, 2, "array")) {
    struct Array *M = lunum_checkarray1(L, 2);
    if (M->dtype != ARRAY_TYPE_BOOL) {
      luaL_error(L, "index array must be of type bool");
    }
    struct Array B = array_new_from_mask(A, M);
    lunum_pusharray1(L, &B);
    return 1;
  }
  else if (lua_type(L, 2) == LUA_TTABLE || lua_type(L, 2) == LUA_TSTRING) {

    lua_getglobal(L, "lunum");
    lua_getfield(L, -1, "__build_slice");
    lua_remove(L, -2);
    lua_pushvalue(L, 1);
    lua_pushvalue(L, 2);
    lua_call(L, 2, 1);

    return 1;
  }

  const int m = _get_index(L, A);
  _push_value(L, A->dtype, (char*)A->data + array_sizeof(A->dtype)*m);

  return 1;
}


int luaC_array__newindex(lua_State *L)
{
  struct Array *A = lunum_checkarray1(L, 1);
  const int m = _get_index(L, A);

  const enum ArrayType T = A->dtype;

  void *val = lunum_tovalue(L, T);
  memcpy((char*)A->data + array_sizeof(T)*m, val, array_sizeof(T));
  free(val);

  return 0;
}

int luaC_array__add(lua_State *L) { return _array_binary_op1(L, ARRAY_OP_ADD); }
int luaC_array__sub(lua_State *L) { return _array_binary_op1(L, ARRAY_OP_SUB); }
int luaC_array__mul(lua_State *L) { return _array_binary_op1(L, ARRAY_OP_MUL); }
int luaC_array__div(lua_State *L) { return _array_binary_op1(L, ARRAY_OP_DIV); }
int luaC_array__pow(lua_State *L) { return _array_binary_op1(L, ARRAY_OP_POW); }
int luaC_array__unm(lua_State *L) { _unary_func(L, runm, 0); return 1; }


int _array_binary_op1(lua_State *L, enum ArrayOperation op)
{
  if (!lunum_hasmetatable(L, 1, "array")) {
    struct Array *B = lunum_checkarray1(L, 2);
    lunum_upcast(L, 1, B->dtype, B->size);
    lua_replace(L, 1);
    struct Array *A = lunum_checkarray1(L, 1);
    array_resize(A, B->shape, B->ndims);
  }
  if (!lunum_hasmetatable(L, 2, "array")) {
    struct Array *A = lunum_checkarray1(L, 1);
    lunum_upcast(L, 2, A->dtype, A->size);
    lua_replace(L, 2);
    struct Array *B = lunum_checkarray1(L, 2);
    array_resize(B, A->shape, A->ndims);
  }
  return _array_binary_op2(L, op);
}

int _array_binary_op2(lua_State *L, enum ArrayOperation op)
{
  struct Array *A = lunum_checkarray1(L, 1);
  struct Array *B = lunum_checkarray1(L, 2);
  int d;


  if (A->ndims != B->ndims) {
    luaL_error(L, "arrays have different dimensions");
  }
  for (d=0; d<A->ndims; ++d) {
    if (A->shape[d] != B->shape[d]) {
      luaL_error(L, "arrays shapes do not agree");
    }
  }


  const int N = A->size;
  enum ArrayType T = (A->dtype >= B->dtype) ? A->dtype : B->dtype;

  struct Array A_ = (A->dtype == T) ? *A : array_new_copy(A, T);
  struct Array B_ = (B->dtype == T) ? *B : array_new_copy(B, T);

  struct Array C = array_new_zeros(N, T);
  array_resize(&C, A->shape, A->ndims);
  lunum_pusharray1(L, &C);

  array_binary_op(&A_, &B_, &C, op);

  luaL_getmetatable(L, "array");
  lua_setmetatable(L, -2);

  if (A->dtype != T) array_del(&A_);
  if (B->dtype != T) array_del(&B_);

  return 1;
}



int luaC_lunum_array(lua_State *L)
{
  if (lua_type(L, 2) == LUA_TSTRING) {
    const enum ArrayType T = array_typeflag(lua_tostring(L, 2)[0]);
    lunum_upcast(L, 1, T, 1);
  }
  else {
    const enum ArrayType T = (enum ArrayType) luaL_optinteger(L, 2, ARRAY_TYPE_DOUBLE);
    lunum_upcast(L, 1, T, 1);
  }
  return 1;
}

int luaC_lunum_zeros(lua_State *L)
{
  if (lua_isnumber(L, 1)) {
    const int N = luaL_checkinteger(L, 1);
    const enum ArrayType T = (enum ArrayType) luaL_optinteger(L, 2, ARRAY_TYPE_DOUBLE);
    struct Array A = array_new_zeros(N, T);
    lunum_pusharray1(L, &A);
    return 1;
  }
  else if (lua_istable(L, 1) || lunum_hasmetatable(L, 1, "array")) {

    int d, Nd;
    int *N = (int*) lunum_checkarray2(L, 1, ARRAY_TYPE_INT, &Nd);
    const enum ArrayType T = (enum ArrayType) luaL_optinteger(L, 2, ARRAY_TYPE_DOUBLE);

    int ntot = 1;
    for (d=0; d<Nd; ++d) ntot *= N[d];
    struct Array A = array_new_zeros(ntot, T);

    array_resize(&A, N, Nd);
    lunum_pusharray1(L, &A);

    return 1;
  }
  else {
    luaL_error(L, "argument must be either number, table, or array");
    return 0;
  }
}

int luaC_lunum_range(lua_State *L)
{
  int i;
  const int N = luaL_checkinteger(L, 1);
  struct Array A = array_new_zeros(N, ARRAY_TYPE_INT);
  lunum_pusharray1(L, &A);
  for (i=0; i<N; ++i) {
    ((int*)A.data)[i] = i;
  }
  return 1;
}

int luaC_lunum_resize(lua_State *L)
{
  int d, Nd;
  struct Array *A = lunum_checkarray1(L, 1); // the array to resize
  int *N = (int*) lunum_checkarray2(L, 2, ARRAY_TYPE_INT, &Nd);

  int ntot = 1;
  for (d=0; d<Nd; ++d) ntot *= N[d];

  if (A->size != ntot) {
    luaL_error(L, "new and old total sizes do not agree");
    return 0;
  }
  array_resize(A, N, Nd);

  return 0;
}

int luaC_lunum_slice(lua_State *L)
{

  // The first part of this function extracts a slice of the array 'A' according
  // to the convention start:stop:skip. The result is a contiguous array 'B'
  // having the same number of dimensions as 'A'.
  // ---------------------------------------------------------------------------
  int d, e, Nd0, Nd1, Nd2, Nd3;

  const struct Array *A = lunum_checkarray1(L, 1); // the array to resize
  int *start   = (int*) lunum_checkarray2(L, 2, ARRAY_TYPE_INT, &Nd0);
  int *stop    = (int*) lunum_checkarray2(L, 3, ARRAY_TYPE_INT, &Nd1);
  int *skip    = (int*) lunum_checkarray2(L, 4, ARRAY_TYPE_INT, &Nd2);
  int *squeeze = (int*) lunum_checkarray2(L, 5, ARRAY_TYPE_INT, &Nd3);


  if (Nd0 != A->ndims || Nd1 != A->ndims || Nd2 != A->ndims || Nd3 != A->ndims) {
    luaL_error(L, "slice has wrong number of dimensions for array");
  }

  for (d=0; d<A->ndims; ++d) {
    if (start[d] < 0 || stop[d] > A->shape[d]) {
      luaL_error(L, "slice not within array extent");
    }
  }
  struct Array B = array_new_from_slice(A, start, stop, skip, Nd0);


  // The rest of this function deals with squeezing out the size-1 dimensions of
  // 'B' which are marked by the 'squeeze' array.
  // ---------------------------------------------------------------------------
  int Nd_new = 0;
  for (d=0; d<Nd0; ++d) Nd_new += !squeeze[d];

  // In case we're left with a 0-dimensional (scalar) slice
  if (Nd_new == 0) {
    _push_value(L, B.dtype, B.data);
    return 1;
  }
  // In case there are any dims to squeeze out
  else if (Nd_new != Nd0) {

    int *shape_new = (int*) malloc(Nd_new * sizeof(int));
    for (d=0,e=0; d<Nd0; ++d) {
      if (B.shape[d] > 1 || !squeeze[d]) {
	shape_new[e] = B.shape[d];
	++e;
      }
    }
    array_resize(&B, shape_new, Nd_new);
    free(shape_new);
  }

  lunum_pusharray1(L, &B);
  return 1;
}



int luaC_lunum_sin(lua_State *L) { _unary_func(L, sin, 1); return 1; }
int luaC_lunum_cos(lua_State *L) { _unary_func(L, cos, 1); return 1; }
int luaC_lunum_tan(lua_State *L) { _unary_func(L, tan, 1); return 1; }

int luaC_lunum_asin(lua_State *L) { _unary_func(L, asin, 1); return 1; }
int luaC_lunum_acos(lua_State *L) { _unary_func(L, acos, 1); return 1; }
int luaC_lunum_atan(lua_State *L) { _unary_func(L, atan, 1); return 1; }

int luaC_lunum_sinh(lua_State *L) { _unary_func(L, sinh, 1); return 1; }
int luaC_lunum_cosh(lua_State *L) { _unary_func(L, cosh, 1); return 1; }
int luaC_lunum_tanh(lua_State *L) { _unary_func(L, tanh, 1); return 1; }

int luaC_lunum_exp(lua_State *L) { _unary_func(L, exp, 1); return 1; }
int luaC_lunum_log(lua_State *L) { _unary_func(L, log, 1); return 1; }
int luaC_lunum_log10(lua_State *L) { _unary_func(L, log10, 1); return 1; }


int luaC_lunum_loadtxt(lua_State *L)
// -----------------------------------------------------------------------------
// Opens the text file 'fname' for reading, and parses the data
// line-by-line. It is assumed that the data is all floating point, and that
// only a space is used as a separator. If there are multiple columns then a 2d
// array is created. All rows must have the same number of entries, otherwise an
// error is generated.
// -----------------------------------------------------------------------------
{
  const char *fname = luaL_checkstring(L, 1);
  FILE *input = fopen(fname, "r");

  if (input == NULL) {
    luaL_error(L, "no such file %s", fname);
  }

  int nline = 0;
  int ncols = 0;
  int ntot = 0;
  double *data = NULL;

  char line[2048];

  while (fgets(line, sizeof(line), input)) {

    if (strlen(line) == 1) {
      continue;
    }

    int nvals = 0;
    double *vals = NULL;
    char *word = strtok(line, " \n");

    while (word) {
      vals = (double*) realloc(vals, ++nvals*sizeof(double));
      vals[nvals-1] = atof(word);
      word = strtok(NULL, " \n");
    }

    if (ncols == 0) ncols = nvals;
    if (ncols != nvals) {
      luaL_error(L, "wrong number of data on line %d of %s", nline, fname);
    }

    data = (double*) realloc(data, (ntot+=nvals)*sizeof(double));
    memcpy(data+ntot-nvals, vals, nvals*sizeof(double));
    free(vals);

    ++nline;
  }
  fclose(input);

  lunum_pusharray2(L, data, ARRAY_TYPE_DOUBLE, ntot);
  struct Array *A = lunum_checkarray1(L, -1);

  int shape[2] = { nline, ncols };
  array_resize(A, shape, ncols == 1 ? 1 : 2);

  free(data);
  return 1;
}


int luaC_lunum_fromfile(lua_State *L)
// -----------------------------------------------------------------------------
// Opens the binary file 'fname' for reading, and returns a 1d array from the
// data. The file size must be a multiple of the data type 'T'.
// -----------------------------------------------------------------------------
{
  const char *fname = luaL_checkstring(L, 1);
  const enum ArrayType T = luaL_optinteger(L, 2, ARRAY_TYPE_DOUBLE);
  const int sizeof_T = array_sizeof(T);

  FILE *input = fopen(fname, "rb");

  if (input == NULL) {
    luaL_error(L, "no such file %s", fname);
  }
  fseek(input, 0L, SEEK_END); const int sz = ftell(input);
  fseek(input, 0L, SEEK_SET);

  if (sz % sizeof_T != 0) {
    luaL_error(L, "file size must be a multiple of the data type size");
  }
  const int N = sz / sizeof_T;
  struct Array A = array_new_zeros(N, T);

  fread(A.data, N, sizeof_T, input);
  fclose(input);
  lunum_pusharray1(L, &A);

  return 1;
}


#define EXPR_EVALF(T,N,x) {int i; for(i=0;i<N;++i)((T*)(x))[i]=f(((T*)(x))[i]);}
#define EXPR_EVALG(T,N,x) {int i; for(i=0;i<N;++i)((T*)(x))[i]=g(((T*)(x))[i]);}

void _unary_func(lua_State *L, double(*f)(double), int cast)
{
  if (lua_isnumber(L, 1)) {
    const double x = lua_tonumber(L, 1);
    lua_pushnumber(L, f(x));
  }
  else if (lunum_hasmetatable(L, 1, "array")) {
    struct Array *A = (struct Array*) lunum_checkarray1(L, 1);

    if (cast == 0) {
      struct Array B = array_new_copy(A, A->dtype);

      switch (B.dtype) {
      case ARRAY_TYPE_BOOL    : EXPR_EVALF(Bool   , B.size, B.data); break;
      case ARRAY_TYPE_CHAR    : EXPR_EVALF(char   , B.size, B.data); break;
      case ARRAY_TYPE_SHORT   : EXPR_EVALF(short  , B.size, B.data); break;
      case ARRAY_TYPE_INT     : EXPR_EVALF(long   , B.size, B.data); break;
      case ARRAY_TYPE_LONG    : EXPR_EVALF(int    , B.size, B.data); break;
      case ARRAY_TYPE_FLOAT   : EXPR_EVALF(float  , B.size, B.data); break;
      case ARRAY_TYPE_DOUBLE  : EXPR_EVALF(double , B.size, B.data); break;
      }

      lunum_pusharray1(L, &B);
    }
    else if (A->dtype <= ARRAY_TYPE_DOUBLE) {
      struct Array B = array_new_copy(A, ARRAY_TYPE_DOUBLE);
      double *b = (double*) B.data;
      int i;
      for (i=0; i<B.size; ++i) b[i] = f(b[i]);
      lunum_pusharray1(L, &B);
    }
  }
}

int _get_index(lua_State *L, struct Array *A)
{
  int m = 0;

  if (lua_isnumber(L, 2)) {
    m = luaL_checkinteger(L, 2);

    if (m >= A->size || m < 0) {
      luaL_error(L, "index %d out of bounds on array of length %d", m, A->size);
    }
  }
  else if (lua_istable(L, 2)) {
    int d, Nd;
    int *ind = (int*) lunum_checkarray2(L, 2, ARRAY_TYPE_INT, &Nd);

    if (A->ndims != Nd) {
      luaL_error(L, "wrong number of indices (%d) on array of dimension %d",
                 Nd, A->ndims);
    }
    int *stride = (int*) malloc(A->ndims * sizeof(int));
    stride[Nd-1] = 1;

    for (d=Nd-2; d>=0; --d) {
      stride[d] = stride[d+1] * A->shape[d+1];
    }

    for (d=0; d<A->ndims; ++d) {
      if (ind[d] >= A->shape[d] || ind[d] < 0) {
        luaL_error(L, "array indexed out of bounds (%d) on dimension %d of size %d",
                   ind[d], d, A->shape[d]);
      }
      m += ind[d]*stride[d];
    }
    free(stride);
  }
  return m;
}


void _push_value(lua_State *L, enum ArrayType T, void *v)
{
  switch (T) {
  case ARRAY_TYPE_BOOL    : lua_pushboolean(L,    *((Bool   *)v)); break;
  case ARRAY_TYPE_CHAR    : lua_pushnumber (L,    *((char   *)v)); break;
  case ARRAY_TYPE_SHORT   : lua_pushnumber (L,    *((short  *)v)); break;
  case ARRAY_TYPE_INT     : lua_pushnumber (L,    *((int    *)v)); break;
  case ARRAY_TYPE_LONG    : lua_pushnumber (L,    *((long   *)v)); break;
  case ARRAY_TYPE_FLOAT   : lua_pushnumber (L,    *((float  *)v)); break;
  case ARRAY_TYPE_DOUBLE  : lua_pushnumber (L,    *((double *)v)); break;
  }
}

