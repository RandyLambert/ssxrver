#ifndef SSERVER_SINGLETON_H
#define SSERVER_SINGLETON_H

#include <stdlib.h> // atexit
#include <pthread.h>
#include <assert.h>
namespace sserver
{

namespace detail
{
// This doesn't detect inherited member functions!
// http://stackoverflow.com/questions/1966362/sfinae-to-check-for-inherited-member-functions
template<typename T>
struct has_no_destroy
{
  template <typename C> static char test(typeof(&C::no_destroy)); // or decltype in C++11
  template <typename C> static int32_t test(...);
  const static bool value = sizeof(test<T>(0)) == 1;
};
}

template<typename T>
class Singleton 
{
 public:
  static T& instance()
  {
    pthread_once(&ponce_, &Singleton::init);//能够保证这个对象只被调用一次，还能保证线程安全
    assert(value_ != NULL);
    return *value_;
  }
  Singleton(const Singleton &) = delete ;
  Singleton& operator=(const Singleton &) = delete ;


 private:
  Singleton();
  ~Singleton();

  static void init()
  {
    value_ = new T();//在init方法中创建
    if (!detail::has_no_destroy<T>::value)
    {
      ::atexit(destroy);//程序结束后会自动销毁，在整个程序结束的时候会自动调用这个函数销毁
    }
  }

  static void destroy()//销毁
  {
    typedef char T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];//对象必须是一个complate态的，这样的话就存在风险，通过typedef定义
                                                                    //，因为数组不能是为-1，如果有问题编译阶段就发现了
    T_must_be_complete_type dummy; (void) dummy;

    delete value_;
    value_ = NULL;
  }

 private:
  static pthread_once_t ponce_;
  static T*             value_;
};

template<typename T>
pthread_once_t Singleton<T>::ponce_ = PTHREAD_ONCE_INIT;

template<typename T>
T* Singleton<T>::value_ = NULL;

}
#endif //SSERVER_SINGLETON_H
