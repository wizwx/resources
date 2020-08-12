#include <future>
#include <iostream>
#include <functional>
#include <stdint.h>
#include <vector>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>
#include <sys/sysinfo.h>

#define NUM_THREADS 1

struct Benchmark {
		std::chrono::high_resolution_clock clock;
		std::chrono::high_resolution_clock::time_point before;

		void Start() { before = clock.now(); }

		uint64_t Stop() {
			auto after = clock.now();
			auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(after - before).count();
			return duration;
		} //Stop
};

typedef void (*fn_t)(uint64_t);

struct Arg {
  fn_t fn;
  uint64_t cnt;
};

std::atomic<uint64_t> start{0};

void * wrapper(void * v) {
  Arg * arg = (Arg*)v;
  int ready;
  do {
    sleep(0);
    ready = start.load(std::memory_order_acquire);
  } while (!ready);
  /*
  while (!start) {
    sleep(0);
  }
  */
  fn_t fn = arg->fn;
  Benchmark bm;
  bm.Start();
  (*fn)(arg->cnt);
  return (void *)bm.Stop();
}

struct taslock {
  std::atomic<bool> lock_ = {0};

  void lock() {
    for (;;) {
      if (!lock_.exchange(true, std::memory_order_acquire)) {
        break;
      }
      while (lock_.load(std::memory_order_relaxed)) {
        __builtin_ia32_pause();
      }
    }
  }    

/*
  void lock() { 
    while(lock_.exchange(true, std::memory_order_acquire)); 
  }
*/

  void unlock() { lock_.store(false, std::memory_order_release); }
};

std::atomic<uint64_t> a{0};
std::atomic<uint64_t> b{0};
volatile uint64_t c = 0;
volatile uint64_t d = 0;
volatile uint64_t e = 0;
volatile uint64_t f = 0;
volatile uint64_t g = 0;
volatile uint64_t h = 0;

static pthread_spinlock_t lock;
static taslock lock2;

__attribute__((constructor))
void lock_constructor () {
    if ( pthread_spin_init ( &lock, 0 ) != 0 ) {
        exit ( 1 );
    }
}

void loop1(uint64_t limit) {
  for (uint64_t i = 0; i < limit; ++i) {
    ++a;
  }
}

void loop2(uint64_t limit) {
  for (uint64_t i = 0; i < limit; ++i) {
    b.fetch_add(1, std::memory_order_relaxed);
  }
}

void loop3(uint64_t limit) {
  for (uint64_t i = 0; i < limit; ++i) {
    __atomic_fetch_add(&c, 1, __ATOMIC_RELAXED);
  }
}

void loop4(uint64_t limit) {
  for (uint64_t i = 0; i < limit; ++i) {
    __atomic_fetch_add(&d, 1, __ATOMIC_SEQ_CST);
  }
}

void loop5(uint64_t limit) {
  for (uint64_t i = 0; i < limit; ++i) {
    pthread_spin_lock(&lock);
    ++e;
    pthread_spin_unlock(&lock);
  }
}

void loop6(uint64_t limit) {
  uint64_t v;
  for (uint64_t i = 0; i < limit; ++i) {
    do {
      v = f;
    } while (!__atomic_compare_exchange_n(&f, &v, v+1, false, __ATOMIC_RELEASE, __ATOMIC_RELAXED));
  }
}

void loop7(uint64_t limit) {
  for (uint64_t i = 0; i < limit; ++i) {
    ++g;
  }
}

void loop8(uint64_t limit) {
  for (uint64_t i = 0; i < limit; ++i) {
    lock2.lock();
    ++h;
    lock2.unlock();
  }
}

void benchmark(uint64_t iteration, void (*fn)(uint64_t)) {
  Arg arg{fn, iteration};
  //start = 0;
  start.store(0, std::memory_order_release);
  sleep(1);

  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  pthread_t threads[NUM_THREADS];
  for (int t=0; t<NUM_THREADS; ++t) {
    int rc = pthread_create(&threads[t], &attr, wrapper, &arg);
    cpu_set_t  mask;
    CPU_ZERO(&mask);
    CPU_SET(t, &mask);
    pthread_setaffinity_np(threads[t], sizeof(cpu_set_t), &mask);
  }
  
  //start = 1;
  start.store(1, std::memory_order_release);
  pthread_attr_destroy(&attr);
  void * status;
  for(int t=0; t<NUM_THREADS; ++t) {
    int rc = pthread_join(threads[t], &status);
    if (rc) {
      printf("ERROR; return code from pthread_join() is %d\n", rc);
      exit(-1);
    }
    std::cout << "time=" << ((uint64_t)status * 1./iteration) << std::endl;
  }
}

int main() {
  uint64_t iteration = 20000000;

  std::cout << "atomic ++" << std::endl;
  benchmark(iteration, loop1);
  std::cout << "value=" << a << "\n" << std::endl;

  std::cout << "atomic fetch_add" << std::endl;
  benchmark(iteration, loop2);
  std::cout << "value=" << b << "\n" << std::endl;

  std::cout << "gcc fetch_add relaxed" << std::endl;
  benchmark(iteration, loop3);
  std::cout << "value=" << c << "\n" << std::endl;

  std::cout << "gcc fetch_add seq_cst" << std::endl;
  benchmark(iteration, loop4);
  std::cout << "value=" << d << "\n" << std::endl;

  std::cout << "spinlock" << std::endl;
  benchmark(iteration, loop5);
  std::cout << "value=" << e << "\n" << std::endl;

  std::cout << "spinlock2" << std::endl;
  benchmark(iteration, loop8);
  std::cout << "value=" << h << "\n" << std::endl;

  std::cout << "gcc cas intrinsic" << std::endl;
  benchmark(iteration, loop6);
  std::cout << "value=" << f << "\n" << std::endl;

  std::cout << "none" << std::endl;
  benchmark(iteration, loop7);
  std::cout << "value=" << g << "\n" << std::endl;
} 

__attribute__((destructor))
void lock_destructor () {
    if ( pthread_spin_destroy ( &lock ) != 0 ) {
        exit ( 3 );
    }
}
