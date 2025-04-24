#include <dbg.h>
#include <skutils/macro.h>

#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/map.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/interprocess_fwd.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/named_condition.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/interprocess/sync/named_semaphore.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <cstddef>

namespace bip = boost::interprocess;

// (1) ManagedSharedMemory 基本使用，与具名对象的构造和查找
static void Demo_ConstructPrimitiveTypeOnSharedMemory() {
    // 创建内存块
    bip::shared_memory_object::remove("PrimitiveShm");
    bip::managed_shared_memory segment(bip::open_or_create, "PrimitiveShm", 1024);

    // 构造类型，指定一个名字后面可以根据此查找
    bool*        pbool     = segment.construct<bool>("Bool")();
    int*         pint      = segment.construct<int>("Int")(100);
    int*         pintarr   = segment.construct<int>("IntArr")[10]();
    char*        pchar     = segment.construct<char>("Char")('a');
    char*        pchararr  = segment.construct<char>("CharArr")[10]('a');
    const char** pcharchar = segment.construct<const char*>("CharChar")("aaa");

    // 查找或直接构建
    double* pdouble = segment.find_or_construct<double>("Double")(3.14);

    // 查找类型，返回结果是，T*和一个size，
    //* 其中size用于标明这到底是一个指针还是一个指针数组
    std::pair<int*, std::size_t>         fint      = segment.find<int>("Int");
    std::pair<int*, std::size_t>         fintarr   = segment.find<int>("IntArr");
    std::pair<char*, std::size_t>        fchararr  = segment.find<char>("CharArr");
    std::pair<const char**, std::size_t> fcharchar = segment.find<const char*>("CharChar");

    dbg("fintarr type: ", dbg::type<decltype(fintarr)>(), fintarr.second);        // 10, 10个元素
    dbg("fchararr type: ", dbg::type<decltype(fchararr)>(), fchararr.second);     // 10，10个元素
    dbg("fcharchar type: ", dbg::type<decltype(fcharchar)>(), fcharchar.second);  // 0, 说明不是数组

    dbg(fint.first, *(fint.first));
    dbg(fintarr.first, fintarr.first[2]);

    // 显式销毁具名对象
    segment.destroy<bool>("Bool");
    segment.destroy<int>("Int");
    segment.destroy<int>("IntArr");
    segment.destroy<char>("Char");
    segment.destroy<char>("CharArr");
    segment.destroy<const char*>("CharChar");
    segment.destroy<double>("Double");
}

// (2) 如何在 Managed_Shared_Memory 上构造任意类型
//      => 只需要做到，指定好分配器，让东西分配到共享内存上（而不是堆上）就好
static void Demo_ConstructVectorOnSharedMemory() {
    // 1. 创建共享内存
    bip::shared_memory_object::remove("VecShm");
    bip::managed_shared_memory segment(bip::open_or_create, "VecShm", 1024);

    // 2. 定制分配器
    using IntAllocator = bip::allocator<int, bip::managed_shared_memory::segment_manager>;
    using SharedVector = bip::vector<int, IntAllocator>;

    // 3. 根据分配器创建数据结构
    IntAllocator  alloc(segment.get_segment_manager());
    SharedVector* parr = segment.construct<SharedVector>("Vec")(alloc);  //* 这里传一个分配器
    int*          pint = segment.construct<int>("Int")(100);

    //! 这里不断push_back，也不会导致 pint 被覆盖。
    // 因为 managed_shared_memory 内部其实是维护了一个内存池（而不是连续内存），parr和pint互不相干，
    // vec扩容的时候，会自动找一个合适的内存块分配给它，而不是覆盖pint
    parr->push_back(1);
    parr->push_back(3);
    parr->push_back(2);
    parr->push_back(1);

    dbg(parr);
    dbg(parr->data());  // parr 和 parr->data() 可不是一个地址
    dbg(pint);
}

// (3) Mutex同步。 注意是跨进程同步，所以一般的锁只有同进程的线程可以看到
//      1. 具名锁，由操作系统管理，不存储在共享内存中，需要手动销毁
//      2. 无名锁，存储在共享内存中，内存销毁的时候，自动销毁
static void Demo_LockOfSharedMemory() {
    bip::shared_memory_object::remove("LockShm");
    bip::managed_shared_memory segment(bip::open_or_create, "LockShm", 1024);
    bip::named_mutex::remove("NamedMutex1");

    // 创建具名锁
    bip::named_mutex named_mutex(bip::open_or_create, "NamedMutex1");
    named_mutex.lock();
    named_mutex.unlock();
    bip::named_mutex::remove("NamedMutex1");

    // 创建匿名锁
    //* 注意这个匿名是相对于其他进程来说的，它虽然有名字，但是存储在共享内存中，不是全局可见的
    bip::interprocess_mutex* anony_mutex = segment.find_or_construct<bip::interprocess_mutex>("AnonymousMutex1")();
    anony_mutex->lock();
    anony_mutex->unlock();

    // 内存销毁时匿名锁自动销毁
    bip::shared_memory_object::remove("LockShm");
}

// （4）其他同步。除了mutex，还有一堆别的
static void Demo_SyncOfSharedMemory() {
    bip::shared_memory_object::remove("SyncShm");
    bip::managed_shared_memory segment(bip::open_or_create, "SyncShm", 1024);

    // 各种同步工具
    bip::named_mutex                   mtx(bip::open_or_create, "NamedMutex1");
    bip::scoped_lock<bip::named_mutex> lock(mtx);
    bip::named_condition               cond(bip::open_or_create, "NamedCond1");
    bip::named_semaphore               sem(bip::open_or_create, "NamedSem1", 1);

    bip::shared_memory_object::remove("SyncShm");
    bip::named_mutex::remove("NamedMutex1");
    bip::named_condition::remove("NamedCond1");
    bip::named_semaphore::remove("NamedSem1");
}

int main() {
    RUN_DEMO(Demo_ConstructPrimitiveTypeOnSharedMemory);
    RUN_DEMO(Demo_ConstructVectorOnSharedMemory);
    RUN_DEMO(Demo_LockOfSharedMemory);
    RUN_DEMO(Demo_SyncOfSharedMemory);
}