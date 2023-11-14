#include <cxxabi.h>
#include <cstdio>
#include <cstdlib>
#include <backtrace.h>

extern "C" void init_back_trace(const char *filename);
extern "C" void print_back_trace();

void *__bt_state = nullptr;

int bt_callback(void *, uintptr_t, const char *filename, int lineno, const char *function) {
  /// demangle function name
  const char *func_name = function;
  int status;
  char *demangled = abi::__cxa_demangle(function, nullptr, nullptr, &status);
  if (status == 0) {
    func_name = demangled;
  }

  /// print
  printf("%s:%d in function %s\n", filename, lineno, func_name);
  return 0;
}

void bt_error_callback(void *, const char *msg, int errnum) {
  printf("Error %d occurred when getting the stacktrace: %s", errnum, msg);
}

void bt_error_callback_create(void *, const char *msg, int errnum) {
  printf("Error %d occurred when initializing the stacktrace: %s", errnum, msg);
}

void init_back_trace(const char *filename) {
  __bt_state = backtrace_create_state(filename, 0, bt_error_callback_create, nullptr);
}

void print_back_trace() {
  if (!__bt_state) { /// make sure init_back_trace() is called
    printf("Make sure init_back_trace() is called before calling print_stack_trace()\n");
    abort();
  }
  backtrace_full((backtrace_state *) __bt_state, 0, bt_callback, bt_error_callback, nullptr);
}

namespace ns {

template <typename T, typename U>
void foo(T t, U u) {
  print_back_trace(); // <-------- backtrace here!
}

}  // namespace ns

template <typename T>
struct Klass {
  T t;
  void bar() {
    ns::foo(t, true);
  }
};

int main(int argc, char** argv) {
  init_back_trace(argv[0]);
  Klass<double> k;
  k.bar();

  return 0;
}
