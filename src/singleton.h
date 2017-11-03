#ifndef __SINGLETON_H__
#define __SINGLETON_H__

template< typename T >
  class Singleton
  {
  public:
    static T& instance()
    {
      static T singleton;
      return singleton;
    }

  private:
    Singleton();
  };

#endif//__SINGLETON_H__
