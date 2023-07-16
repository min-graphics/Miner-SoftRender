#pragma once
#include <atomic>
/*
����һ��������(SpinLock)���࣬��ʹ����C++11��ԭ�Ӳ���(std::atomic)��ʵ��ͬ����
����������Ҫ˼�������ٽ����ڵȴ����̲߳��ᱻ���𣬶��ǲ��ϵ�ִ��һ��æ�ȴ���ѭ����ֱ����ȡ������

������˵�������е� lock() ����ʵ�����������ļ�����������ͨ�� std::atomic<bool>::compare_exchange_weak() ������ʵ��ԭ�Ӳ�����
compare_exchange_weak() ������Ƚ� flag_ �� expect ��ֵ�����������ȣ��� flag_ ��Ϊ true ������ true����ʾ�����ɹ���
������ǲ���ȣ��� expect ��Ϊ true ������ false����ʾ����ʧ�ܣ���Ҫ���������ȴ���
��Ҫע����ǣ��ڼ���ʧ�ܺ���Ҫ�� expect ��ԭ������ expect ��ֵ���ܻ����δ����������

unlock() ����ʵ�����������Ľ������������� flag_ ��Ϊ false����ʾ��ǰ���Ѿ��ͷţ������߳̿��Ի�ȡ����

�����е� flag_ ��Ա������һ�� std::atomic<bool> ���ͣ�������ʾ����״̬������һ��ԭ�����ͣ����Ա�֤���̲߳���ʱ����ȷ�ԡ�
�ڹ��캯���У��� flag_ �ĳ�ʼֵ��Ϊ false����ʾ���ĳ�ʼ״̬Ϊδ��ռ�á�

*/
class SpinLock {
public:
	SpinLock() : flag_(false)
	{}
	void lock()
	{
		bool expect = false;
		//���flag��expect��ֵ��������򷵻�true���Ұ�flag��Ϊtrue�� 
		//��������򷵻�false���Ұ�expect��Ϊtrue(����ÿ��ѭ��һ��Ҫ��expect��ԭ)
		while (!flag_.compare_exchange_weak(expect, true))
		{
			//����һ��Ҫ��expect��ԭ; ִ��ʧ��ʱexpect�����δ����
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