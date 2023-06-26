
#ifndef __SERIALIZE_H__
#define __SERIALIZE_H__

#include <iostream>
#include <type_traits>
#include <map>
#include <unordered_map>
#include <string>

template <typename T>
class HasMethod_Serialize
{
private:
	typedef char YesType[1];
	typedef char NoType[2];
	template <typename C> static YesType& test(decltype(&C::Serialize));
	template <typename C> static NoType& test(...);
public:
	enum { value = sizeof(test<T>(0)) == sizeof(YesType) };
};
template <typename T>
class HasMethod_Deserialize
{
private:
	typedef char YesType[1];
	typedef char NoType[2];
	template <typename C> static YesType& test(decltype(&C::Deserialize));
	template <typename C> static NoType& test(...);
public:
	enum { value = sizeof(test<T>(0)) == sizeof(YesType) };
};

template<typename T>
typename std::enable_if<HasMethod_Serialize<T>::value, bool>::type cweeSerialize(T& in, cweeStr& out, int option = -1);
template<typename T>
typename std::enable_if<!HasMethod_Serialize<T>::value, bool>::type cweeSerialize(T& in, cweeStr& out, int option = -1);
template<typename T>
typename std::enable_if<HasMethod_Deserialize<T>::value, bool>::type cweeDeserialize(T& out, cweeStr& in);
template<typename T>
typename std::enable_if<!HasMethod_Deserialize<T>::value, bool>::type cweeDeserialize(T& out, cweeStr& in);

template<typename T>
typename std::enable_if<HasMethod_Serialize<T>::value, bool>::type cweeSerialize(T& in, cweeStr& out, int option) {
	if (option == -1)	out = in.Serialize();
	else out = in.Serialize(option);
	return true; // a class with Serialize() method
}
template<typename T>
typename std::enable_if<!HasMethod_Serialize<T>::value, bool>::type cweeSerialize(T& in, cweeStr& out, int option) {
	return false; // a class without Serialize() method
}
template<typename T>
typename std::enable_if<HasMethod_Deserialize<T>::value, bool>::type cweeDeserialize(T& out, cweeStr& in) {
	out.Deserialize(in);
	return true; // a class with Deserialize() method
}
template<typename T>
typename std::enable_if<!HasMethod_Deserialize<T>::value, bool>::type cweeDeserialize(T& out, cweeStr& in) {	
	return false; // a class without Deserialize() method
}























template <typename T>
class HasMethod_GetName
{
private:
	typedef char YesType[1];
	typedef char NoType[2];
	template <typename C> static YesType& test(decltype(&C::GetName));
	template <typename C> static NoType& test(...);
public:
	enum { value = sizeof(test<T>(0)) == sizeof(YesType) };
};

template<typename T>
typename std::enable_if<HasMethod_GetName<T>::value, bool>::type cweeGetNameMethod(T& in, cweeStr& out);
template<typename T>
typename std::enable_if<!HasMethod_GetName<T>::value, bool>::type cweeGetNameMethod(T& in, cweeStr& out);

template<typename T>
typename std::enable_if<HasMethod_GetName<T>::value, bool>::type cweeGetNameMethod(T& in, cweeStr& out) {
	out = in.GetName();
	return true; // a class with GetName() method
}
template<typename T>
typename std::enable_if<!HasMethod_GetName<T>::value, bool>::type cweeGetNameMethod(T& in, cweeStr& out) {
	return false; // a class without GetName() method
}






















template <typename T>
class HasNameVariable
{
private:
	typedef char YesType[1];
	typedef char NoType[2];
	template <typename C> static YesType& test(decltype(&C::Name));
	template <typename C> static NoType& test(...);
public:
	enum { value = sizeof(test<T>(0)) == sizeof(YesType) };
};

template<typename T>
typename std::enable_if<HasNameVariable<T>::value, bool>::type cweeGetName(T& in, cweeStr& out);
template<typename T>
typename std::enable_if<!HasNameVariable<T>::value, bool>::type cweeGetName(T& in, cweeStr& out);

template<typename T>
typename std::enable_if<HasNameVariable<T>::value, bool>::type cweeGetName(T& in, cweeStr& out) {
	out = in.Name;
	return true; // successful
}
template<typename T>
typename std::enable_if<!HasNameVariable<T>::value, bool>::type cweeGetName(T& in, cweeStr& out) {
	return cweeGetNameMethod(in, out); // try to call the "GetName()" method instead
	// return false; // not successful
}







template <typename T>
class HasManualPolicyCompletionTriggerVariable
{
private:
	typedef char YesType[1];
	typedef char NoType[2];
	template <typename C> static YesType& test(decltype(&C::ManualPolicyCompletionTrigger));
	template <typename C> static NoType& test(...);
public:
	enum { value = sizeof(test<T>(0)) == sizeof(YesType) };
};

template<typename T>
typename std::enable_if<HasManualPolicyCompletionTriggerVariable<T>::value, bool>::type cweeGetManualPolicyCompletionTrigger(T& in, bool& out);
template<typename T>
typename std::enable_if<!HasManualPolicyCompletionTriggerVariable<T>::value, bool>::type cweeGetManualPolicyCompletionTrigger(T& in, bool& out);

template<typename T>
typename std::enable_if<HasManualPolicyCompletionTriggerVariable<T>::value, bool>::type cweeGetManualPolicyCompletionTrigger(T& in, bool& out) {
	out = in.ManualPolicyCompletionTrigger;
	return true; // successful
}
template<typename T>
typename std::enable_if<!HasManualPolicyCompletionTriggerVariable<T>::value, bool>::type cweeGetManualPolicyCompletionTrigger(T& in, bool& out) {
	return false; // not successful
}


#define has_attribute(attributeName)										\
template <typename T>														\
class has_attributeName														\
{																			\
	typedef char one;														\
	struct two { char x[2]; };												\
	template <typename C> static one test(decltype(&C::attributeName));		\
	template <typename C> static two test(...);								\
public:																		\
	enum { value = sizeof(test<T>(0)) == sizeof(char) };					\
	explicit operator bool() const { return value; }						\
}

has_attribute(x);

/*
has_attribute(attributeNameHere); // returns true if the class has x in it. Used like this: 
...
vec3 temp;
bool doesItHaveIt = has_attributeNameHere<vec3>::value;
if (true) 
{
	it has it!
}
else 
{
	it does not have it!
}

*/











#endif