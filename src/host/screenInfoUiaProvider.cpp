/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "precomp.h"

#include "screenInfoUiaProvider.hpp"
#include "screenInfo.hpp"

#include "windowUiaProvider.hpp"
#include "window.hpp"

// A helper function to create a SafeArray Version of an int array of a specified length
SAFEARRAY * BuildIntSafeArray(_In_reads_(length) const int * data, _In_ int length)
{
    SAFEARRAY *sa = SafeArrayCreateVector(VT_I4, 0, length);
    if (sa == NULL)
    {
        return NULL;
    }

    for (long i = 0; i < length; i++)
    {
        if (FAILED(SafeArrayPutElement(sa, &i, (void *)&(data[i]))))
        {
            SafeArrayDestroy(sa);
            sa = NULL;
            break;
        }
    }

    return sa;
}

ScreenInfoUiaProvider::ScreenInfoUiaProvider(_In_ Window* const pParent, _In_ SCREEN_INFORMATION* const pScreenInfo) :
    _pWindow(pParent),
    _pScreenInfo(pScreenInfo),
    _cRefs(1)
{
}

ScreenInfoUiaProvider::~ScreenInfoUiaProvider()
{

}

#pragma region IUnknown

IFACEMETHODIMP_(ULONG) ScreenInfoUiaProvider::AddRef()
{
    return InterlockedIncrement(&_cRefs);
}

IFACEMETHODIMP_(ULONG) ScreenInfoUiaProvider::Release()
{
    long val = InterlockedDecrement(&_cRefs);
    if (val == 0)
    {
        delete this;
    }
    return val;
}

IFACEMETHODIMP ScreenInfoUiaProvider::QueryInterface(_In_ REFIID riid, _COM_Outptr_result_maybenull_ void** ppInterface)
{
    if (riid == __uuidof(IUnknown))
    {
        *ppInterface = static_cast<IRawElementProviderSimple*>(this);
    }
    else if (riid == __uuidof(IRawElementProviderSimple))
    {
        *ppInterface = static_cast<IRawElementProviderSimple*>(this);
    }
    else if (riid == __uuidof(IRawElementProviderFragment))
    {
        *ppInterface = static_cast<IRawElementProviderFragment*>(this);
    }
    else
    {
        *ppInterface = NULL;
        return E_NOINTERFACE;
    }

    (static_cast<IUnknown*>(*ppInterface))->AddRef();

    return S_OK;
}

#pragma endregion

#pragma region IRawElementProviderSimple

// Implementation of IRawElementProviderSimple::get_ProviderOptions.
// Gets UI Automation provider options.
IFACEMETHODIMP ScreenInfoUiaProvider::get_ProviderOptions(_Out_ ProviderOptions* pOptions)
{
    *pOptions = ProviderOptions_ServerSideProvider;
    return S_OK;
}

// Implementation of IRawElementProviderSimple::get_PatternProvider.
// Gets the object that supports ISelectionPattern.
IFACEMETHODIMP ScreenInfoUiaProvider::GetPatternProvider(_In_ PATTERNID patternId, _COM_Outptr_result_maybenull_ IUnknown** ppInterface)
{
    UNREFERENCED_PARAMETER(patternId);

    *ppInterface = NULL;

    // TODO: MSFT: 7960168 - insert patterns here 

    return S_OK;
}

// Implementation of IRawElementProviderSimple::get_PropertyValue.
// Gets custom properties.
IFACEMETHODIMP ScreenInfoUiaProvider::GetPropertyValue(_In_ PROPERTYID propertyId, _Out_ VARIANT* pVariant)
{
    pVariant->vt = VT_EMPTY;

    // Returning the default will leave the property as the default
    // so we only really need to touch it for the properties we want to implement
    if (propertyId == UIA_ControlTypePropertyId)
    {
        // This control is the Document control type, implying that it is
        // a complex document that supports text pattern
        pVariant->vt = VT_I4;
        pVariant->lVal = UIA_DocumentControlTypeId;
    }
    else if (propertyId == UIA_NamePropertyId)
    {
        // In a production application, this would be localized text.
        pVariant->bstrVal = SysAllocString(L"Text Area");
        if (pVariant->bstrVal != NULL)
        {
            pVariant->vt = VT_BSTR;
        }
    }
    else if (propertyId == UIA_AutomationIdPropertyId)
    {
        pVariant->bstrVal = SysAllocString(L"Text Area");
        if (pVariant->bstrVal != NULL)
        {
            pVariant->vt = VT_BSTR;
        }
    }
    else if (propertyId == UIA_IsControlElementPropertyId)
    {
        pVariant->vt = VT_BOOL;
        pVariant->boolVal = VARIANT_TRUE;
    }
    else if (propertyId == UIA_IsContentElementPropertyId)
    {
        pVariant->vt = VT_BOOL;
        pVariant->boolVal = VARIANT_TRUE;
    }
    else if (propertyId == UIA_IsKeyboardFocusablePropertyId)
    {
        pVariant->vt = VT_BOOL;
        pVariant->boolVal = VARIANT_FALSE;
    }
    else if (propertyId == UIA_HasKeyboardFocusPropertyId)
    {
        pVariant->vt = VT_BOOL;
        pVariant->boolVal = VARIANT_FALSE;
    }
    else if (propertyId == UIA_ProviderDescriptionPropertyId)
    {
        pVariant->bstrVal = SysAllocString(L"Microsoft Console Host: Screen Information Text Area");
        if (pVariant->bstrVal != NULL)
        {
            pVariant->vt = VT_BSTR;
        }
    }

    return S_OK;
}

IFACEMETHODIMP ScreenInfoUiaProvider::get_HostRawElementProvider(_COM_Outptr_result_maybenull_ IRawElementProviderSimple** ppProvider)
{
    *ppProvider = NULL;

    return S_OK;
}
#pragma endregion

#pragma region IRawElementProviderFragment

IFACEMETHODIMP ScreenInfoUiaProvider::Navigate(_In_ NavigateDirection direction, _COM_Outptr_result_maybenull_ IRawElementProviderFragment** ppProvider)
{
    *ppProvider = NULL;

    if (direction == NavigateDirection_Parent)
    {
        *ppProvider = new WindowUiaProvider(_pWindow);
        RETURN_IF_NULL_ALLOC(*ppProvider);
    }

    // For the other directions the default of NULL is correct
    return S_OK;
}

IFACEMETHODIMP ScreenInfoUiaProvider::GetRuntimeId(_Outptr_result_maybenull_ SAFEARRAY** ppRuntimeId)
{
    // Root defers this to host, others must implement it...
    *ppRuntimeId = NULL;

    // AppendRuntimeId is a magic Number that tells UIAutomation to Append its own Runtime ID(From the HWND)
    int rId[] = { UiaAppendRuntimeId, -1 };
    // BuildIntSafeArray is a custom function to hide the SafeArray creation
    *ppRuntimeId = BuildIntSafeArray(rId, 2);
    RETURN_IF_NULL_ALLOC(*ppRuntimeId);

    return S_OK;
}

IFACEMETHODIMP ScreenInfoUiaProvider::get_BoundingRectangle(_Out_ UiaRect* pRect)
{
    RETURN_HR_IF_NULL((HRESULT)UIA_E_ELEMENTNOTAVAILABLE, _pWindow);

    RECT const rc = _pWindow->GetWindowRect();

    pRect->left = rc.left;
    pRect->top = rc.top;
    pRect->width = rc.right - rc.left;
    pRect->height = rc.bottom - rc.top;

    return S_OK;
}

IFACEMETHODIMP ScreenInfoUiaProvider::GetEmbeddedFragmentRoots(_Outptr_result_maybenull_ SAFEARRAY** ppRoots)
{
    *ppRoots = NULL;
    return S_OK;
}

IFACEMETHODIMP ScreenInfoUiaProvider::SetFocus()
{
    return S_OK;
}

IFACEMETHODIMP ScreenInfoUiaProvider::get_FragmentRoot(_COM_Outptr_result_maybenull_ IRawElementProviderFragmentRoot** ppProvider)
{
    *ppProvider = new WindowUiaProvider(_pWindow);
    RETURN_IF_NULL_ALLOC(*ppProvider);
    return S_OK;
}

#pragma endregion
