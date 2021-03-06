#include <map>
#include <memory>
#include <set>
#include <vector>
#include <stdint.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mcheck.h>
#include "allocator.h"
using namespace std;

class RegisterItem {
	void* protect;
	multiset<RegisterItem*> sub_objects;
	typedef multiset<RegisterItem*>::iterator RegisterItemIterator;
public:
	RegisterItem() {
	}
	RegisterItem(void * ptr) :
			protect(ptr), sub_objects() {
	}
	void add_depency(RegisterItem *obj) {
		sub_objects.insert(obj);
	}
	void remove_depency(RegisterItem * ptr) {
		auto itr = sub_objects.find(ptr);
		if (itr != sub_objects.end()) {
			sub_objects.erase(itr);
		}
	}
	RegisterItemIterator depency_begin() {
		return sub_objects.begin();
	}
	RegisterItemIterator depency_end() {
		return sub_objects.end();
	}
};

static const size_t COLLECT_BOUND = 500;
static const size_t collect_stack_size = 500000;
static const size_t collect_redzone_offset = 480000;

static map<void*, RegisterItem *> registry;
static multiset<RegisterItem*> roots;
static set<tuple<void*, void*> > roots_ref;
static set<RegisterItem *> reachable;
static set<RegisterItem *> incomplete;
static CachedAllocator alloc(40000, 10, 10);
static void * collect_stack;
static void * collect_stack_redzone;

static bool is_valid(int t, void* p) {
	return t && registry.count(p);
}

static void clean_ptr(void * ptr) {
	auto it = registry.find(ptr);
	if (it != registry.end()) {
		delete it->second;
		registry.erase(it);
		alloc.deallocate(ptr);
	}
}

static void collect_stack_check() {
	register long esp asm ("esp");
	if (esp < (long)collect_stack_redzone)
		throw std::runtime_error("collect stack overflows\n");

}

static void collect_dfs(RegisterItem * root) {
	collect_stack_check();
	if (!incomplete.count(root) && reachable.count(root))
		return;
	incomplete.insert(root);
	reachable.insert(root);
	for(auto item = root->depency_begin(); item != root->depency_end(); item++) {
		 RegisterItem * reg_item = *item;
		 if (!incomplete.count(reg_item))
			 collect_dfs(reg_item);
	}
	incomplete.erase(root);
}

static void run_collect_dfs(RegisterItem * root) {
	incomplete.insert(root);
	while (!incomplete.empty()) {
		RegisterItem * next = *incomplete.begin();
		try {
			collect_dfs(next);
		} catch (...) {}
	}
}

extern "C" void gc_collect(int full) {
	vector<void *> clean;
	reachable.clear();
	if (!full) {
		for(auto it : roots) {
			run_collect_dfs(it);
		}
		for(auto it : roots_ref) {
			int t = *((int*)get<0>(it));
			void* p = *((void**)get<1>(it));
			if (is_valid(t, p)) {
				run_collect_dfs(registry[p]);
			}
		}
	}
	for(auto it : registry) {
		RegisterItem * item = it.second;
		if (!reachable.count(item)) {
			clean.push_back(it.first);
		}
	}
	for(auto ptr : clean) {
		clean_ptr(ptr);
	}
}

extern "C"  {
	void switchme(void *, int);
}

static void run_gc_collect(int full) {
	switchme((char*)collect_stack + collect_stack_size - 1, full);
}

extern void* gc_malloc(size_t size) {
	void * ptr = NULL;
	try {
		ptr = alloc.allocate(size);
	} catch(...) {
		ptr = NULL;
		run_gc_collect(0);
		try {
			ptr = alloc.allocate(size);
		} catch(...) {
			ptr = NULL;
		}
	}
	if (ptr == NULL)
		return NULL;
	RegisterItem * item = new RegisterItem(ptr);
	registry[ptr] = item;
	return ptr;
}


extern "C" {

extern void Tgc_init() {
	collect_stack = malloc(collect_stack_size);
	collect_stack_redzone = (char*)collect_stack + collect_redzone_offset;
}

extern void Tgc_make_root(int t, void* p) {
	if (is_valid(t, p)) {
		roots.insert(registry[p]);
	}
}

extern void Tgc_remove_root(int t, void *p) {
	if (is_valid(t, p)) {
		auto it = roots.find(registry[p]);
		if (it != roots.end())
			roots.erase(it);
	}
}
extern void Tgc_make_root_ref(void * t, void *p) {
	roots_ref.insert(make_tuple(t, p));
}

extern void Tgc_remove_root_ref(void * t, void *p) {
	auto it = roots_ref.find(make_tuple(t, p));
	if (it != roots_ref.end())
		roots_ref.erase(it);
}

extern void Tgc_ref(void* a, int nt, void* n) {
	if (is_valid(nt, n)) {
		registry[a]->add_depency(registry[n]);
	}
}

extern void Tgc_ping(int full) {
	if (alloc.last_memory_allocated() >= COLLECT_BOUND || full) {
		run_gc_collect(full);
		alloc.clear_last_memory_counter();
		if (full) {
			if (mprobe(collect_stack) == MCHECK_OK)
			free(collect_stack);
		}
	}
}


}
