#pragma once

#include "crazygaze/core/Singleton.h"

#if defined(_WIN32)
	MLGE_THIRD_PARTY_INCLUDES_START
	#include <d3d11.h>
	#include <d3d12.h>
	#include <dxgidebug.h>
	#include <dxgi1_3.h>
	#include <wrl/client.h>
	#include <comdef.h>
	MLGE_THIRD_PARTY_INCLUDES_END
#endif

namespace mlge
{

#if defined(_WIN32)

	/**
	 * Returns the ref counter of a COM object. 
	 * The counter is calculated by doing a AddRef followed by a Release (which gives us the new counter)
	 */
	inline int getRefCount(IUnknown* ptr)
	{
		ptr->AddRef();
		return static_cast<int>(ptr->Release());
	}

	template<typename Type>
	int getRefCount(Microsoft::WRL::ComPtr<Type>& ptr)
	{
		return getRefCount(ptr.Get());
	}

	// #RVF_VIDEO Remove this. Intentionally using a different name so I can easily find all of them.
	#define MLGE_DX_REFCOUNT_ASSERT(obj, expectedCount)                                                        \
		{                                                                                                      \
			int count = getRefCount(obj);                                                                      \
			if (count != expectedCount)                                                                        \
			{                                                                                                  \
				CZ_LOG(Fatal, "{}: Expected a ref count of {} but was {}", m_debugName, expectedCount, count); \
			}                                                                                                  \
		}
#endif

/**
 * Provided the DX/D3D debug layer.
 * This required the `-d3ddebug` command line parameter. If that is not specified, all calls are noops
 */
class DXDebugLayer : public Singleton<DXDebugLayer>
{
  public:
	DXDebugLayer();
	~DXDebugLayer();

	/**
	 * Initializes the DX debug layer (if available and enabled)
	 *
	 * If the `-d3ddebug` command line parameter is not specified, this function is a noop and returns true.
	 */
	bool init();

	void shutdown();

	void setD3DDebug(SDL_Renderer& renderer);
  private:

	bool isEnabled() const
	{
		return m_dxgiInfoQueue ? true : false;
	}

	Microsoft::WRL::ComPtr<IDXGIInfoQueue> m_dxgiInfoQueue;
	Microsoft::WRL::ComPtr<IDXGIDebug> m_dxgiDebug;
	Microsoft::WRL::ComPtr<ID3D11Debug> m_d3d11Debug;

	Microsoft::WRL::ComPtr<ID3D12Debug> m_d3d12Debug;
	Microsoft::WRL::ComPtr<ID3D12DebugDevice> m_d3d12DebugDevice;
};

} // namespace mlge


