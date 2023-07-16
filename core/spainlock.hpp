#pragma once
#include <atomic>
/*
这是一个自旋锁(SpinLock)的类，它使用了C++11的原子操作(std::atomic)来实现同步。
自旋锁的主要思想是在临界区内等待的线程不会被挂起，而是不断地执行一个忙等待的循环，直到获取到锁。

具体来说，该类中的 lock() 函数实现了自旋锁的加锁操作，它通过 std::atomic<bool>::compare_exchange_weak() 方法来实现原子操作。
compare_exchange_weak() 方法会比较 flag_ 和 expect 的值，如果它们相等，则将 flag_ 置为 true 并返回 true，表示加锁成功；
如果它们不相等，则将 expect 置为 true 并返回 false，表示加锁失败，需要继续自旋等待。
需要注意的是，在加锁失败后，需要将 expect 复原，否则 expect 的值可能会出现未定义的情况。

unlock() 函数实现了自旋锁的解锁操作，它将 flag_ 置为 false，表示当前锁已经释放，其他线程可以获取锁。

该类中的 flag_ 成员变量是一个 std::atomic<bool> 类型，用来表示锁的状态，它是一个原子类型，可以保证多线程操作时的正确性。
在构造函数中，将 flag_ 的初始值设为 false，表示锁的初始状态为未被占用。

*/
class SpinLock {
public:
	SpinLock() : flag_(false)
	{}
	void lock()
	{
		bool expect = false;
		//检查flag和expect的值，若相等则返回true，且把flag置为true， 
		//若不相等则返回false，且把expect置为true(所以每次循环一定要将expect复原)
		while (!flag_.compare_exchange_weak(expect, true))
		{
			//这里一定要将expect复原; 执行失败时expect结果是未定的
			expect = false;
		}
	}
	void unlock()
	{
		flag_.store(false);
	}

private:
	std::atomic<bool> flag_;
};