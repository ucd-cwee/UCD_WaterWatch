#pragma once
#include "Any.h"

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
	};;
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
	std::weak_ptr<Type_Info> const& AnyData::GetTypeShared() const { return user_type_shared<void>(); };
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
	Any& Any::operator=(const Any& rhs) noexcept {
		if (this == &rhs) { return *this; }

		auto locked{ std::unique_lock(mut) };
		auto locked2{ std::shared_lock(rhs.mut) };

		container = rhs.container;
		return *this;
	};
	Any& Any::operator=(Any&& rhs) noexcept {
		auto locked{ std::unique_lock(mut) };
		auto locked2{ std::shared_lock(rhs.mut) };

		container = rhs.container;
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
		static auto DynamicTypeHash{ GetHash(user_type<DynamicObject>()) };
		static auto VarHash{ GetHash(user_type<Var>()) };
		auto locked{ std::shared_lock(mut) };
		if (std::shared_ptr<AnyData>& m = container) {
			if (m->GetTypeHash() == DynamicTypeHash) {
				if (auto p2 = m->cast< DynamicObject>()) {
					return p2->m_actualType;
				}
			}
#ifdef AllowInlineVarTyping
			//if (m->GetTypeHash() == VarHash) {
			if (auto p2 = m->cast< Var>()) {
				if (!p2->p_data->IsEmpty()) {
					return p2->p_data->ActualType();
				}
				else {
					return m->GetTypeShared();
				}
			}
			//}
#endif
			return m->GetTypeShared();
		}
		else {
			return user_type_shared<void>();
		}
	};
	std::weak_ptr<Type_Info> Any::Type() const noexcept {
		static auto DynamicTypeHash{ GetHash(user_type<DynamicObject>()) };
		static auto VarHash{ GetHash(user_type<Var>()) };
		auto locked{ std::shared_lock(mut) };
		if (std::shared_ptr<AnyData>& m = container) {
			if (m->GetTypeHash() == DynamicTypeHash) {
				if (auto p2 = m->cast< DynamicObject>()) {
					return p2->m_classType;
				}
			}
#ifdef AllowInlineVarTyping
			//if (m->GetTypeHash() == VarHash) {
			if (auto* p2 = m->cast< Var>()) {
				if (!p2->p_data->IsEmpty()) {
					return p2->p_data->Type();
				}
				else {
					return m->GetTypeShared();
				}
			}
			//}
#endif
			return m->GetTypeShared();
		}
		else {
			return user_type_shared<void>();
		}
	};
	size_t Any::TypeHash() const noexcept {
		static auto DynamicTypeHash{ GetHash(user_type<DynamicObject>()) };
		static auto VarHash{ GetHash(user_type<Var>()) };
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
			//if (m->GetTypeHash() == VarHash) {
			if (auto* p2 = m->cast< Var>()) {
				if (!p2->p_data->IsEmpty()) {
					return p2->p_data->TypeHash();
				}
				else {
					return m->GetTypeHash();
				}
			}
			//}
#endif
			return m->GetTypeHash();
		}
		else {
			static auto SharedT{ GetHash(user_type<void>()) };
			return SharedT;
		}
	};
	bool Any::IsTypeOf(std::weak_ptr<Type_Info> const& targetType) const noexcept {
		static auto hasher{ std::hash<std::weak_ptr<Type_Info>>() };
		return TypeHash() == hasher(targetType);
	};
	bool Any::IsTypeOf(Type_Info const& targetType) const noexcept {
		static auto hasher{ std::hash<Type_Info>() };
		return TypeHash() == hasher(targetType);
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










