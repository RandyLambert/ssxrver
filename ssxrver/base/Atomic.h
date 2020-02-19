#ifndef SSXRVER_BASE_ATOMIC_H
#define SSXRVER_BASE_ATOMIC_H
#include <stdint.h>
#include "noncopyable.h"
namespace ssxrver
{
namespace detail
{
//模拟java原子操作的api，使用这些原子操作要加上-match=cpu-type，锁的开销大于原子操作
template <typename T>
class AtomicIntegerT : noncopyable
{
public:

    AtomicIntegerT()
        : value_(0)
    {
    }

    T get()
    {
        //获取变量
        return __sync_val_compare_and_swap(&value_, 0, 0);
        //原子比较和交换（设置）操作
        //判断value_是否等与0，如果等于0，则将0赋值给value，返回值是value的值+0之前的值
    }

    T getAndAdd(T x)
    {
        return __sync_fetch_and_add(&value_, x);
        //原子自增操作
        //先获取value_的值，将value_的值返回，在将value_的值+x
    }

    T addAndGet(T x)
    {
        return getAndAdd(x) + x;
        //原子自增操作
        //先获取value_的值，将value_+x的值返回，在将value_的值+x
    }

    T incrementAndGet()
    {
        return addAndGet(1); //++i
    }

    T decrementAndGet()
    {
        return addAndGet(-1); //--i
    }

    void add(T x)
    {
        getAndAdd(x); //先获取，在+
    }

    void increment()
    {
        incrementAndGet();
    }

    void decrement()
    {
        decrementAndGet();
    }

    T getAndSet(T newValue)
    {
        return __sync_lock_test_and_set(&value_, newValue); //原子赋值操作
        //先返回原来的值，在把值负为新值
    }
    
private:
    volatile T value_;
    //volatile的作用：作为指令关键字，确保本条指令不会因编译器优化而忽略
    //且要求每次直接读值，简单的说是防止编译器优化
    //
    //当要求使用volatile声明的变量的值的时候，系统总是重新从他所在的内存读取数据
    //而不是使用保存在寄存器中的备份，是他前面的指令刚刚从该出读取过数据，而且读取的数
    //据立刻被保存，防止多线程出问题
};
} // namespace detail

typedef detail::AtomicIntegerT<int32_t> AtomicInt32;
typedef detail::AtomicIntegerT<int64_t> AtomicInt64;
} // namespace ssxrver
#endif
