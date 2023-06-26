#ifndef __INTERLOCKED_GENERIC_H__
#define __INTERLOCKED_GENERIC_H__



#if 1

template< typename type>
class cweeUnpooledInterlocked {
public:
	template< typename type>
	class ExclusiveObject {
	public:
		constexpr ExclusiveObject(const cweeUnpooledInterlocked<type>& mut) : owner(const_cast<cweeUnpooledInterlocked<type>&>(mut)) {};
		~ExclusiveObject() { this->owner.Unlock(); };

		ExclusiveObject() = delete;
		ExclusiveObject(const ExclusiveObject& other) = delete; // : owner(other.owner) {  };
		ExclusiveObject(ExclusiveObject&& other) = delete; // : owner(other.owner) {  };
		ExclusiveObject& operator=(const ExclusiveObject& other) = delete;
		ExclusiveObject& operator=(ExclusiveObject&& other) = delete;

		type& operator=(const type& a) { data() = a; return data(); };
		type& operator=(type&& a) { data() = a; return data(); };
		type& operator*() const { return data(); };
		type* operator->() const { return &data(); };

	protected:
		type& data() const { return *owner.UnsafeRead(); };
		cweeUnpooledInterlocked<type>& owner;
	};


public: // construction and destruction
	typedef type		Type;

	cweeUnpooledInterlocked() : data(new type()), lock() {};
	cweeUnpooledInterlocked(const type& other) : data(new type()), lock() {
		Swap(other);
	};
	cweeUnpooledInterlocked(type&& other) : data(new type()), lock() {
		Swap(std::forward<type>(other));
	};
	cweeUnpooledInterlocked(const cweeUnpooledInterlocked& other) : data(new type()), lock() {
		this->Copy(other); // instantiate by copying provided object
	};
	cweeUnpooledInterlocked(cweeUnpooledInterlocked&& other) : data(new type()), lock() {
		this->Copy(std::forward<cweeUnpooledInterlocked>(other)); // instantiate by copying provided object
	};
	~cweeUnpooledInterlocked() {};

public: // copy and clear
	cweeUnpooledInterlocked<type>& operator=(const cweeUnpooledInterlocked<type>& other) {
		this->Copy(other);
		return *this;
	};
	cweeUnpooledInterlocked<type>& operator=(cweeUnpooledInterlocked<type>&& other) {
		this->Copy(std::forward<cweeUnpooledInterlocked<type>>(other));
		return *this;
	};
	void Copy(const cweeUnpooledInterlocked<type>& copy) {
		if (&copy == this) return;
		copy.Lock();
		Lock();
		{
			*data = *copy.data;
		}
		Unlock();
		copy.Unlock();
	};
	void Copy(cweeUnpooledInterlocked<type>&& copy) {
		if (&copy == this) return;
		copy.Lock();
		Lock();
		{
			*data = *copy.data;
		}
		Unlock();
		copy.Unlock();
	};
	void Clear() {
		Lock();
		{
			data = cweeSharedPtr<type>(new type());
		}
		Unlock();
	};

public: // read and swap
	type Read() {
		type out;
		Lock();	
		auto ptr = UnsafeRead();
		if (ptr) {
			out = *ptr;
		}
		else {
			out = type();
		}
		Unlock();
		return out;
	};
	type Read() const {
		type out;
		Lock();
		auto ptr = UnsafeRead();
		if (ptr) {
			out = *ptr;
		}
		else {
			out = type();
		}
		Unlock();
		return out;
	};
	void Swap(const type& replacement) {
		Lock();
		auto ptr = UnsafeRead();
		if (ptr) {
			*ptr = replacement;
		}
		Unlock();
	};
	cweeUnpooledInterlocked<type>& operator=(const type& other) {
		Swap(other);
		return *this;
	};
	operator type() {
		return Read();
	};
	operator type() const {
		return Read();
	};

	cweeSharedPtr<type> operator->() {
		return data;
	};
	cweeSharedPtr<type> operator->() const {
		return data;
	};

public: // lock, unlock, and direct edit
	ExclusiveObject<type> GetExclusive() const {
		Lock();
		return ExclusiveObject<type>(*this);
	};

	AUTO Guard() const {
		return lock.Guard();
	};
	bool TryLock() const {
		if (lock.Lock(false) == true) return true;
		return false;
	};
	void Lock() const {
		lock.Lock();		
	};
	void Unlock() const {
		lock.Unlock();		
	};
	type* UnsafeRead() const {
		return data.Get();
	};

private:
	cweeSharedPtr<type> data;
	cweeSysMutex lock;
};

template< typename type>
class cweeInterlocked {
public: // construction and destruction
	typedef type		Type;

	cweeInterlocked() {
		if (data == nullptr) {
			data = new type;
		}
	};
	cweeInterlocked(const type& other) {
		if (data == nullptr) {
			data = new type;
		}
		Swap(other);
	};
	cweeInterlocked(const cweeInterlocked& other) {
		if (data == nullptr) {
			data = new type;
		}
		this->Copy(other); // instantiate by copying provided object
	};
	~cweeInterlocked() {
		if (data != nullptr) {
			delete data;
		}
	};

public: // copy and clear
	cweeInterlocked<type>& operator=(const cweeInterlocked<type>& other) {
		this->Copy(other);
		return *this;
	};
	void Copy(const cweeInterlocked<type>& copy) {
		if (copy.lock.lock.data == lock.lock.data) {
			// sharing the same pooled lock
			Lock();
			{
				*data = *copy.data;
			}
			Unlock();
		}
		else {
			// not the same lock. 
			copy.Lock();
			Lock();
			{
				*data = *copy.data;
			}
			Unlock();
			copy.Unlock();
		}
	};
	void Clear() {
		Lock();
		{
			delete data;
			data = new type;
		}
		Unlock();
	};

public: // read and swap
	type Read() {
		type out;
		Lock();
		out = *data;
		Unlock();
		return out;
	};
	type Read() const {
		type out;
		Lock();
		out = *data;
		Unlock();
		return out;
	};
	void Swap(const type& replacement) {
		Lock();
		*data = replacement;
		Unlock();
	};
	cweeInterlocked<type>& operator=(const type& other) {
		Swap(other);
		return *this;
	};
	operator type() {
		return Read();
	};
	operator type() const {
		return Read();
	};

	std::shared_ptr<type> operator->() {
		Lock();
		std::shared_ptr<type> foo = std::make_shared<type>(*data);
		Unlock();
		return foo;
	};
	std::shared_ptr<type> operator->() const {
		Lock();
		std::shared_ptr<type> foo = std::make_shared<type>(*data);
		Unlock();
		return foo;
	};

public: // lock, unlock, and direct edit
	bool TryLock() const {
		if (lock.Lock(false) == true) return true;
		return false;
	};
	void Lock() const {
		lock.Lock();
	};
	void Unlock() const {
		lock.Unlock();
	};
	type* UnsafeRead() const {
		return data;
	};

private:
	type* data = nullptr;
	// mutable cweeSysMutex lock;
	mutable cweeManagedMutex lock;
};

#else

template< typename type>
class cweeUnpooledInterlocked {
public: // construction and destruction
	cweeUnpooledInterlocked() {
		if (data == nullptr) data = new type;
	};
	cweeUnpooledInterlocked(const type& other) {
		if (data == nullptr) data = new type;
		Swap(other);
	};
	cweeUnpooledInterlocked(const cweeUnpooledInterlocked& other) {
		if (data == nullptr) data = new type;
		this->Copy(other); // instantiate by copying provided object
	};
	~cweeUnpooledInterlocked() {
		if (data != nullptr) delete data;
	};

public: // copy and clear
	cweeUnpooledInterlocked<type>& operator=(const cweeUnpooledInterlocked<type>& other) {
		this->Copy(other);
		return *this;
	};
	void Copy(const cweeUnpooledInterlocked<type>& copy) {
		copy.Lock();
		Lock();
		{
			*data = *copy.data;
		}
		Unlock();
		copy.Unlock();
	};
	void Clear() {
		Lock();
		{
			delete data;
			data = new type;
		}
		Unlock();
	};

public: // read and swap
	type Read() {
		type out;
		Lock();
		out = *data;
		Unlock();
		return out;
	};
	type Read() const {
		type out;
		Lock();
		out = *data;
		Unlock();
		return out;
	};
	void Swap(const type& replacement) {
		Lock();
		*data = replacement;
		Unlock();
	};
	cweeUnpooledInterlocked<type>& operator=(const type& other) {
		Swap(other);
		return *this;
	};
	operator type() {
		return Read();
	};
	operator type() const {
		return Read();
	};

	std::shared_ptr<type> operator->() {
		Lock();
		std::shared_ptr<type> foo = std::make_shared<type>(*data);
		Unlock();
		return foo;
	};
	std::shared_ptr<type> operator->() const {
		Lock();
		std::shared_ptr<type> foo = std::make_shared<type>(*data);
		Unlock();
		return foo;
	};

public: // lock, unlock, and direct edit
	bool TryLock() const {
		if (lock.Lock(false) == true) return true;
		return false;
	};
	void Lock() const {
		lock.Lock();
	};
	void Unlock() const {
		lock.Unlock();
	};
	type* UnsafeRead() const {
		return data;
	};

private:
	type* data = nullptr;
	mutable cweeSysMutex lock;
};

#endif


#endif