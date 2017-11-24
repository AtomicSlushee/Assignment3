#ifndef __SINGLETON_H__
#define __SINGLETON_H__

template< typename T >
  class Singleton
  {
  public:
    template<typename... Args>
    static T& instance(Args... args)
    {
      static T singleton(args...);
      return singleton;
    }

  private:
    Singleton();
  };

#endif//__SINGLETON_H__
