#pragma once
#ifdef WINDOWS

#endif // WINDOWS

#ifdef ANDROID

#endif // ANDROID
#include "utils/IncludeFFmpeg.h"
#include <vector>
#include <mutex>
template <typename T>
class ComponentManager
{
private:
	std::vector<T *> vec;
	std::mutex magMutex;

public:
	ComponentManager();
	T *GetPointerById(int id);
	int PushPointerAndGetid(T *pointer);
	bool DeletePointerByid(int id);
};

template <typename T>
inline ComponentManager<T>::ComponentManager()
{
}

template <typename T>
inline T* ComponentManager<T>::GetPointerById(int id)
{
	magMutex.lock();
	if (id < 0 || id >= vec.size()) return nullptr;
	T *pointer = vec[id];
	if (pointer == nullptr)
		av_log_error("pointer is nullptr,it will cause some error\n");
	magMutex.unlock();
	return pointer;
}

template <typename T>
inline int ComponentManager<T>::PushPointerAndGetid(T *pointer)
{
	magMutex.lock();
	vec.push_back(pointer);
	int result = vec.size() - 1;
	magMutex.unlock();
	return result;
}

template <typename T>
inline bool ComponentManager<T>::DeletePointerByid(int id)
{
	magMutex.lock();
	if (id < 0 || id >= vec.size()) return false;
	T *pointer = vec[id];
	magMutex.unlock();
	delete pointer;
	return true;
}
