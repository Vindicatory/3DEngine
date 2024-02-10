#pragma once

template<typename T>
class PRIMITIVE_DESC {
public:
	inline PRIMITIVE_DESC(T* object)
	{
		mananagedObject = object;
	}

	virtual ~PRIMITIVE_DESC() {
		delete mananagedObject;
	}

	const T* operator->() {
		return mananagedObject;
	}

	T* GetPtr() {
		return mananagedObject;
	}

protected:
   T* mananagedObject;
};