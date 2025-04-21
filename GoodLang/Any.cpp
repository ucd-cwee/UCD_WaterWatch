#pragma once
#include "Any.h"
#include <array>

// Type_Info
namespace GoodLang {
	size_t Type_Info::GetHashImpl() const {
		static size_t result{ impl::TypeId<void>().hash_code() };
		return result;

	};
	void Type_Info::CacheHash() noexcept {
		const_cast<size_t&>(underlyingHash) = this->GetHashImpl();
		const_cast<size_t&>(uniqueHash) = this->underlyingHash;
		details::hash_combine(const_cast<size_t&>(uniqueHash), (size_t)is_const(), (size_t)is_ref());
	};
	size_t Type_Info::GetHash() const {
		return uniqueHash;
	};
	bool Type_Info::CanCast(Type_Info const& from, Type_Info const& to) {
		if (from.underlyingHash == to.underlyingHash) { // underlying matches
			// anything can convert into const T&
			if (to.is_const() && to.is_ref()) return true;

			// const T cannot be cast to T
			if (!to.is_const() && from.is_const()) return false;

			// T cannot be cast to T&
			if (!from.is_ref() && to.is_ref()) return false;

			return true;
		}
		return false;
	};
	bool Type_Info::CanCast(Type_Info const& to) const {
		return CanCast(*this, to);
	};
	bool Type_Info::is_const() const noexcept { return isConst; };;
	bool Type_Info::is_void() const noexcept { return isVoid; };
	bool Type_Info::is_ref() const noexcept { return isRef; };
	bool Type_Info::is_any() const noexcept { return isAny; };
	std::string Type_Info::name() const noexcept { return impl::TypeId<void>().name(); };
	std::weak_ptr<Type_Info> Type_Info::MakeBase() const { return std::weak_ptr<Type_Info>(); };
	std::weak_ptr<Type_Info> Type_Info::MakeConst() const { return std::weak_ptr<Type_Info>(); };
	std::weak_ptr<Type_Info> Type_Info::MakeRef() const { return std::weak_ptr<Type_Info>(); };
	std::weak_ptr<Type_Info> Type_Info::MakeConstRef() const { return std::weak_ptr<Type_Info>(); };
	std::weak_ptr<Type_Info> Type_Info::RemoveConst() const { return std::weak_ptr<Type_Info>(); };
	std::weak_ptr<Type_Info> Type_Info::RemoveRef() const { return std::weak_ptr<Type_Info>(); };
	bool Type_Info::IsBuiltInType() const { return true; };

	size_t Scripted_Type_Info::GetHashImpl() const {
		return this->m_uniqueHash;
	};
	std::string Scripted_Type_Info::name() const noexcept {
		if (is_const()) {
			if (is_ref()) {
				return std::string("const ") + m_name + "&";
			}
			else {
				return std::string("const ") + m_name;
			}
		}
		else {
			if (is_ref()) {
				return m_name + "&";
			}
			else {
				return m_name;
			}
		}

		return m_full_name;
	};
	void Scripted_Type_Info::SetSelf(std::shared_ptr<Scripted_Type_Info>& t_self) {
		m_self = t_self;
	};
	std::weak_ptr<Type_Info> Scripted_Type_Info::MakeBase() const {
		return MakeDuplicate(false, false);
	};
	std::weak_ptr<Type_Info> Scripted_Type_Info::MakeConst() const {
		return MakeDuplicate(true, is_ref());
	};
	std::weak_ptr<Type_Info> Scripted_Type_Info::MakeRef() const {
		return MakeDuplicate(is_const(), true);
	};
	std::weak_ptr<Type_Info> Scripted_Type_Info::MakeConstRef() const {
		return MakeDuplicate(true, true);
	};
	std::weak_ptr<Type_Info> Scripted_Type_Info::RemoveConst() const {
		return MakeDuplicate(false, is_ref());
	};
	std::weak_ptr<Type_Info> Scripted_Type_Info::RemoveRef() const {
		return MakeDuplicate(is_const(), false);
	};
	bool Scripted_Type_Info::IsBuiltInType() const { return false; };
	std::weak_ptr<Type_Info> Scripted_Type_Info::MakeDuplicate(bool targetConst, bool targetRef) const {
		size_t targetHash = this->GetHashImpl();
		details::hash_combine(targetHash, (size_t)targetConst, (size_t)targetRef);

		if (this->GetHash() == targetHash) {
			return m_self;
		}
		else if (auto parentPtr = m_parent.lock()) {
			return parentPtr->MakeDuplicate(targetConst, targetRef);
		}
		else {
			if (1) {
				auto locked{ std::shared_lock(m_children_mut) };
				auto p = m_children.find(targetHash);
				if (p != m_children.end()) {
					return std::dynamic_pointer_cast<Type_Info>(p->second);
				}
			}

			if (1) {
				auto locked{ std::unique_lock(m_children_mut) };
				auto p = m_children.find(targetHash);
				if (p != m_children.end()) {
					return std::dynamic_pointer_cast<Type_Info>(p->second);
				}
				else {
					auto out = std::make_shared<Scripted_Type_Info>(m_qualified_namespace, m_name, targetConst, targetRef);
					out->SetSelf(out);
					out->m_parent = this->m_self;

					this->m_children.insert({ targetHash, out });
					return std::dynamic_pointer_cast<Type_Info>(out);
				}
			}
		}
	};

	template<> size_t GetHash<Type_Info>(Type_Info const& r) {
		return r.GetHash();
	};
	template<> size_t GetHash<std::shared_ptr<Type_Info>>(std::shared_ptr<Type_Info> const& r) {
		static auto voidHash{ GoodLang::impl::TypeId<void>().hash_code() };
		if (r) {
			return r->GetHash();
		}
		else {
			return voidHash;
		}
	};
	template<> size_t GetHash<std::weak_ptr<Type_Info>>(std::weak_ptr<Type_Info> const& r) {
		return GetHash(r.lock());
	};
};

// shared_ptr
namespace GoodLang {
	// std::numeric_limits<unsigned short>::max() and 512 give similar performance metrics.
	// 255 and less tend to get caught in a constriction with heavy loads. 
	static std::array<GoodLang::fast_shared_mutex, std::numeric_limits<unsigned short>::max()> locks; // 128, 1000, 10000, std::numeric_limits<unsigned short>::max()
	static size_t PtrToIndex(shared_ptr_base::aux* const& ptr) {
		if constexpr (locks.size() == std::numeric_limits<unsigned short>::max()) {
			return reinterpret_cast<unsigned short&>(const_cast<shared_ptr_base::aux*&>(ptr));
		}
		else if constexpr (locks.size() == std::numeric_limits<unsigned char>::max()) {
			return reinterpret_cast<unsigned char&>(const_cast<shared_ptr_base::aux*&>(ptr));
		}
		else {
			return (reinterpret_cast<size_t&>(const_cast<shared_ptr_base::aux*&>(ptr)) >> 5) % locks.size();
		}
	};

	void shared_ptr_base::PreventDeletion(aux* const& ptr) {
		if (ptr) locks[PtrToIndex(ptr)].lock_shared();
	};
	void shared_ptr_base::AllowDeletion(aux* const& ptr) {
		if (ptr) locks[PtrToIndex(ptr)].unlock_shared();
	};
	// requires that the ptr is WEAK LOCKED
	void shared_ptr_base::DoDeletion(aux* const& ptr) {
		if (ptr) {
			auto& lock = locks[PtrToIndex(ptr)];
			(void)lock.upgrade_lock();
			delete ptr;
			lock.unlock();
		}
	};
	// requires that the ptr is NOT LOCKED. 
	void shared_ptr_base::DoDestroyOrDelete(aux* const& ptr, bool Destroy, bool Delete) {
		if (ptr) {
			if (Destroy) {
				ptr->destroy(); // the destruction must take place when completely unlocked, since downstream / daisy-chained deletions may take place and need access. 
			}
			if (Delete) {
				auto& lock = locks[PtrToIndex(ptr)];
				lock.lock();
				delete ptr;
				lock.unlock();
			}
		}
	};

	// USER MUST ALLOW DELETION AFTER RECIEVING THE PTR
	shared_ptr_base::aux* shared_ptr_base::inc(GoodLang::atomic_ptr<aux> const& pa) {
		aux
			* pa_ptr{ nullptr },
			* out{ nullptr };
		long long
			read;

		while (pa_ptr = pa.load()) {
			// prevent its deletion while we work on it. This does not access it, it simply locks the region the pointer belongs to, HOPING to prevent collisions. 
			PreventDeletion(pa_ptr);
			if (pa_ptr == (out = pa.load())) {
				if (pa_ptr) {
					read = pa_ptr->Strong_Weak_Destroy_Delete.fetch_add(1, std::memory_order::memory_order_relaxed) + 1; // increments the strong count, regardless of the others
					if (
						(reinterpret_cast<short*>(&read)[0] >= 0)
						&& (reinterpret_cast<long*>(&read)[1] == 0)
						// && (reinterpret_cast<short*>(&read)[2] == 0)
						// && (reinterpret_cast<short*>(&read)[3] == 0)
						) { // if NOT being destroyed or deleted...
						return out; // remember - I am still locked from deletion.
					}
					else {
						pa_ptr->Strong_Weak_Destroy_Delete.fetch_add(-1, std::memory_order::memory_order_acq_rel); // failure -- exit immediately.
					}
				}
			}
			AllowDeletion(pa_ptr);
		}
		return nullptr;
	};
	// ASSUMES THAT THE PTR COMES IN LOCKED.
	void shared_ptr_base::dec(aux* pa_ptr) {
		long long
			read,
			planned;

		if (pa_ptr) {
			read = pa_ptr->Strong_Weak_Destroy_Delete.fetch_add(-1, std::memory_order::memory_order_acq_rel) - 1;
			if ((reinterpret_cast<short*>(&read)[0] < 0) || reinterpret_cast<short*>(&read)[2] || reinterpret_cast<short*>(&read)[3]) {
				// too late! 
				AllowDeletion(pa_ptr);
			}
			else {
				if (reinterpret_cast<short*>(&read)[0] == 0) {
					planned = read;

					reinterpret_cast<short*>(&planned)[0] = -1;
					reinterpret_cast<short*>(&planned)[2] = 1;
					if (reinterpret_cast<short*>(&planned)[1] == 0) {
						// flag that we plan on deleting the mem_block!
						reinterpret_cast<short*>(&planned)[3] = 1;
					}
					if (pa_ptr->Strong_Weak_Destroy_Delete.compare_exchange_weak(read, planned, std::memory_order::memory_order_acq_rel)) { // success!
						AllowDeletion(pa_ptr);
						if ((reinterpret_cast<short*>(&planned)[2] == 1) || (reinterpret_cast<short*>(&planned)[3] == 1))
							DoDestroyOrDelete(
								pa_ptr,
								reinterpret_cast<short*>(&planned)[2] == 1,
								reinterpret_cast<short*>(&planned)[3] == 1
							);
						return;
					}
				}
				// still good
				AllowDeletion(pa_ptr);
			}
		}
	};
	// USER MUST ALLOW DELETION AFTER RECIEVING THE PTR
	shared_ptr_base::aux* shared_ptr_base::inc_weak(GoodLang::atomic_ptr<aux> const& pa) {
		aux
			* pa_ptr{ nullptr },
			* pa_ptr_copy{ nullptr },
			* out{ nullptr };
		long long
			read,
			planned;

		while (pa_ptr_copy = pa_ptr = pa.load()) {
			// prevent its deletion while we work on it. This does not access it, it simply locks the region the pointer belongs to, HOPING to prevent collisions. 
			PreventDeletion(pa_ptr_copy);
			if (pa_ptr_copy == (out = pa_ptr = pa.load())) {
				if (pa_ptr) {
					planned = read = pa_ptr->Strong_Weak_Destroy_Delete.load(std::memory_order::memory_order_relaxed);
					if (!reinterpret_cast<short*>(&read)[3]) { // if NOT being destroyed or deleted...
						// add to the weak count
						++reinterpret_cast<short*>(&planned)[1];
						if (pa_ptr->Strong_Weak_Destroy_Delete.compare_exchange_weak(read, planned, std::memory_order::memory_order_acq_rel)) { // success!
							return out; // remember - I am still locked from deletion.
						}
					}
				}
			}
			AllowDeletion(pa_ptr_copy);
		}
		return nullptr;
	};
	// ASSUMES THAT THE PTR COMES IN LOCKED.
	void shared_ptr_base::dec_weak(aux* pa_ptr) {
		long long
			read,
			planned;

		if (pa_ptr) {
			planned = read = pa_ptr->Strong_Weak_Destroy_Delete.load(std::memory_order::memory_order_relaxed);
			if (!reinterpret_cast<short*>(&read)[3]) { // if NOT being destroyed or deleted...
				--reinterpret_cast<short*>(&planned)[1];
				if (reinterpret_cast<short*>(&planned)[1] == 0) {
					// flag that we plan on deleting the mem_block!
					reinterpret_cast<short*>(&planned)[3] = 1;
				}
				if (pa_ptr->Strong_Weak_Destroy_Delete.compare_exchange_weak(read, planned, std::memory_order::memory_order_acq_rel)) { // success!
					// AllowDeletion(pa_ptr);
					if (reinterpret_cast<short*>(&planned)[3] == 1) {
						DoDeletion(pa_ptr);
					}
					return;
				}
			}
			AllowDeletion(pa_ptr);
		}
	};
};

// Any, AnyAutoCast, DynamicObject, exceptions
namespace GoodLang {
	// AnyData
	void AnyData::SetSelf(std::shared_ptr< AnyData>& t_self) {
		m_self = t_self;
		typeHash = GetHash(GetType());
	};
	size_t AnyData::GetTypeHash() const { return typeHash; };
	bool AnyData::CanCast(Type_Info const& to_type) const { return false; };
	Type_Info const& AnyData::GetType() const { return user_type<void>(); };
	std::weak_ptr<Type_Info> const& AnyData::GetTypeShared() const { 
		return user_type_shared<void>(); 
	};
	std::shared_ptr<Type_Info> const& AnyData::GetTypeSharedPtr() const { 
		return user_type_shared_ptr<void>();
	};
	void* AnyData::ptr() const { return nullptr; };
	std::shared_ptr<void> AnyData::shared_ptr() const { return nullptr; };
	void AnyData::ThrowIfNot(Type_Info const& type) const {
		if (!CanCast(type)) {
			throw exception::bad_any_cast(GetType(), type, __LINE__);
		}
	};
	bool AnyData::GetFlag(Flag which) const {
		return m_flags[static_cast<int>(which)];
	};
	void AnyData::SetFlag(Flag which, bool newV) {
		m_flags[static_cast<int>(which)] = newV;
	};

	// Any
	Any& Any::swap(Any& rhs) noexcept {
		if (this == &rhs) { return *this; }

		auto locked{ std::unique_lock(mut) };
		auto locked2{ std::unique_lock(rhs.mut) };

		container.swap(rhs.container);
		return *this;
	};
	Any& Any::swap(Any&& rhs) noexcept {
		auto locked{ std::unique_lock(mut) };
		container = std::move(rhs.container);
		return *this;
	};
	Any& Any::operator=(const Any& rhs) noexcept {
		if (this == &rhs) { return *this; }

		auto locked{ std::unique_lock(mut) };
		auto locked2{ std::shared_lock(rhs.mut) };

		container = rhs.container;
		return *this;
	};
	Any& Any::operator=(Any&& rhs) noexcept {
		auto locked{ std::unique_lock(mut) };
		//auto locked2{ std::shared_lock(rhs.mut) };

		container = std::move(rhs.container);
		return *this;
	};
	Any& Any::operator=(std::nullptr_t) noexcept { Clear(); return *this; };
	bool Any::IsEmpty() const noexcept {
		auto locked{ std::shared_lock(mut) };
		if ((bool)container) {
			return container->GetType().is_void();
		}
		else {
			return true;
		}
		// return (bool)container;
	};
	void Any::Clear() noexcept {
		auto locked{ std::unique_lock(mut) };
		container = nullptr;
	};
	std::string Any::TypeName() const noexcept {
		if (auto p = Type().lock()) {
			return p->name();
		}
		else {
			return user_type<void>().name();
		}
	};
	std::weak_ptr<Type_Info> Any::ActualType() const noexcept {
		static auto DynamicTypeHash{ user_type_shared_ptr<DynamicObject>()->GetHash() };
		static auto VarHash{ user_type_shared_ptr<Var>()->GetHash() };
		auto locked{ std::shared_lock(mut) };
		if (std::shared_ptr<AnyData>& m = container) {
			if (m->GetTypeHash() == DynamicTypeHash) {
				if (auto p2 = m->cast< DynamicObject>()) {
					return p2->m_actualType;
				}
			}
#ifdef AllowInlineVarTyping
			if (m->GetTypeHash() == VarHash) {
				if (auto p2 = m->cast< Var>()) {
					if (!p2->p_data->IsEmpty()) {
						return p2->p_data->ActualType();
					}
					else {
						return m->GetTypeShared();
					}
				}
			}
#endif
			return m->GetTypeShared();
		}
		else {
			return user_type_shared<void>();
		}
	};
	std::weak_ptr<Type_Info> Any::Type() const noexcept {
		static auto DynamicTypeHash{ user_type_shared_ptr<DynamicObject>()->GetHash() };
		static auto VarHash{ user_type_shared_ptr<Var>()->GetHash() };
		std::weak_ptr<Type_Info> out{ user_type_shared<void>() };

		mut.lock_shared();		
		if (std::shared_ptr<AnyData>& m = container) {
			if (m->GetTypeHash() == DynamicTypeHash) {
				if (auto* p2 = m->cast< DynamicObject>()) {
					out = p2->m_classType;
					mut.unlock_shared();
					return out;
				}
			}
#ifdef AllowInlineVarTyping
			if (m->GetTypeHash() == VarHash) {
				if (auto* p2 = m->cast< Var>()) {
					if (!p2->p_data->IsEmpty()) {
						out = p2->p_data->Type();
						mut.unlock_shared();
						return out;
					}
				}
			}
#endif
			out = m->GetTypeShared();
		}
		mut.unlock_shared();
		return out;		
	};
	std::shared_ptr<Type_Info> Any::TypePtr() const noexcept {
		static auto DynamicTypeHash{ user_type_shared_ptr<DynamicObject>()->GetHash() };
		static auto VarHash{ user_type_shared_ptr<Var>()->GetHash() };
		std::shared_ptr<Type_Info> out{ user_type_shared_ptr<void>() };

		mut.lock_shared();
		if (std::shared_ptr<AnyData>& m = container) {
			if (m->GetTypeHash() == DynamicTypeHash) {
				if (auto* p2 = m->cast< DynamicObject>()) {
					out = p2->m_classType.lock();
					mut.unlock_shared();
					return out;
				}
			}
#ifdef AllowInlineVarTyping
			if (m->GetTypeHash() == VarHash) {
				if (auto* p2 = m->cast< Var>()) {
					if (!p2->p_data->IsEmpty()) {
						out = p2->p_data->TypePtr();
						mut.unlock_shared();
						return out;
					}
				}
			}
#endif
			out = m->GetTypeSharedPtr();
		}
		mut.unlock_shared();
		return out;
	};
	size_t Any::TypeHash() const noexcept {
		static auto DynamicTypeHash{ user_type_shared_ptr<DynamicObject>()->GetHash() };
		static auto VarHash{ user_type_shared_ptr<Var>()->GetHash() };
		auto locked{ std::shared_lock(mut) };
		if (std::shared_ptr<AnyData>& m = container) {
			if (m->GetTypeHash() == DynamicTypeHash) {
				if (auto p2 = m->cast< DynamicObject>()) {
					if (auto p3 = p2->m_classType.lock()) {
						return p3->GetHash();
					}
				}
			}
#ifdef AllowInlineVarTyping
			if (m->GetTypeHash() == VarHash) {
				if (auto* p2 = m->cast< Var>()) {
					if (!p2->p_data->IsEmpty()) {
						return p2->p_data->TypeHash();
					}
				}
			}
#endif
			return m->GetTypeHash();
		}
		else {
			static auto SharedT{ GetHash(user_type<void>()) };
			return SharedT;
		}
	};
	bool Any::IsTypeOf(std::weak_ptr<Type_Info> const& targetType) const noexcept {
		return TypeHash() == GetHash(targetType);
	};
	bool Any::IsTypeOf(Type_Info const& targetType) const noexcept {
		return TypeHash() == GetHash(targetType);
	};
	std::shared_ptr<AnyData> Any::impl() const {
		auto locked{ std::shared_lock(mut) };
		return container;
	};
	bool Any::GetFlag(AnyData::Flag which) const {
		if (auto m = container) {
			return m->GetFlag(which);
		}
		else {
			return false;
		}
	};
	void Any::SetFlag(AnyData::Flag which, bool newV) {
		if (auto m = container) {
			m->SetFlag(which, newV);
		}
	};

};










