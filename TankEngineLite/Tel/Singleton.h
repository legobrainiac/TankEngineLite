#ifndef SINGLETON_H
#define SINGLETON_H

template <typename T>
class Singleton
{
public:
	static T* GetInstance()
	{
		static T* pInstance{};
        
        if(!pInstance)
            pInstance = new T();
        
		return pInstance;
	}
  
	virtual ~Singleton() = default;
	Singleton(const Singleton& other) = delete;
	Singleton(Singleton&& other) = delete;
	Singleton& operator=(const Singleton& other) = delete;
	Singleton& operator=(Singleton&& other) = delete;

protected:
	Singleton() = default;
};

#endif // !SINGLETON_H