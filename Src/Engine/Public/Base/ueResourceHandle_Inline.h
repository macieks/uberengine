#ifndef UE_RESOURCE_HANDLE_INLINE_H
#define UE_RESOURCE_HANDLE_INLINE_H

template <typename TYPE>
ueResourceHandle<TYPE>::ueResourceHandle() :
	m_resource(NULL)
{}

template <typename TYPE>
ueResourceHandle<TYPE>::~ueResourceHandle()
{
	if (m_resource)
		ueResourceMgr_Release(AsResource());
}

template <typename TYPE>
TYPE* ueResourceHandle<TYPE>::operator * () const
{
	return m_resource;
}

template <typename TYPE>
ueResource* ueResourceHandle<TYPE>::AsResource() const
{
	return (ueResource*) m_resource;
}

template <typename TYPE>
TYPE* ueResourceHandle<TYPE>::operator -> () const
{
	return m_resource;
}

template <typename TYPE>
void ueResourceHandle<TYPE>::operator = (ueResourceHandle<TYPE>& other)
{
	*this = other.m_resource;
}

template <typename TYPE>
void ueResourceHandle<TYPE>::operator = (TYPE* other)
{
	if (m_resource == other)
		return;

	if (m_resource) ueResourceMgr_Release(AsResource());
	m_resource = other;
	if (m_resource) ueResource_IncRefCount(AsResource());
}

template <typename TYPE>
ueBool ueResourceHandle<TYPE>::IsValid() const
{
	return m_resource != NULL;
}

template <typename TYPE>
ueBool ueResourceHandle<TYPE>::IsReady() const
{
	return m_resource && ueResource_IsReady(AsResource());
}

template <typename TYPE>
UE_INLINE ueResourceState ueResourceHandle<TYPE>::GetState() const
{
	return m_resource ? ueResource_GetState(AsResource()) : ueResourceState_Initial;
}

template <typename TYPE>
ueBool ueResourceHandle<TYPE>::SetByName(const char* name, ueBool initImmediately)
{
	if (m_resource)
	{
		if (!ueStrCmp(name, ueResource_GetName(AsResource())))
			return UE_TRUE;
		ueResourceMgr_Release(AsResource());
	}

	m_resource = (TYPE*) ueResourceMgr_Get(TYPE_ID, name, initImmediately);
	if (!m_resource)
		return UE_FALSE;

	ueResourceState state = ueResource_GetState(AsResource());
	return initImmediately ? (state == ueResourceState_Ready) : (state != ueResourceState_FailedToInitialize);
}

template <typename TYPE>
void ueResourceHandle<TYPE>::Release()
{
	*this = NULL;
}

template <typename TYPE>
ueBool ueResourceHandle<TYPE>::Wait()
{
	return m_resource ? ueResource_Wait(AsResource()) : UE_FALSE;
}

template <typename TYPE>
const char* ueResourceHandle<TYPE>::GetName() const
{
	return m_resource ? ueResource_GetName(AsResource()) : NULL;
}

#endif // UE_RESOURCE_HANDLE_INLINE_H