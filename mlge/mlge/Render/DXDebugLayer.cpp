#include "mlge/Render/DXDebugLayer.h"
#include "crazygaze/core/PlatformUtils.h"
#include "mlge/Config.h"

namespace mlge
{

DXDebugLayer::DXDebugLayer()
{
}

DXDebugLayer::~DXDebugLayer()
{
}

bool DXDebugLayer::init()
{
	bool enable = CommandLine::get().has("d3ddebug");

	if (!enable)
	{
		return true;
	}

	SDL_SetHint(SDL_HINT_RENDER_DIRECT3D11_DEBUG, "1");

	{
		auto res = DXGIGetDebugInterface1(0, IID_PPV_ARGS(m_dxgiInfoQueue.GetAddressOf()));
		if (SUCCEEDED(res))
		{
			m_dxgiInfoQueue->SetBreakOnSeverity( DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true );
			m_dxgiInfoQueue->SetBreakOnSeverity( DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true );
		}
		else
		{
			CZ_LOG(Error, "Could not setup d3d debug layer");
			return false;
		}
	}

	DXGIGetDebugInterface1(0, IID_PPV_ARGS(m_dxgiDebug.GetAddressOf()));

	std::string renderApi = Config::get().getValueOrDefault<std::string>("Engine", "renderapi", "direct3d11");
	if (renderApi == "direct3d12")
	{
		HRESULT res = D3D12GetDebugInterface(IID_PPV_ARGS(&m_d3d12Debug));
		if (!SUCCEEDED(res))
		{
			CZ_LOG(Error, "Could not get D3D12Debug interface. Error={}.", _com_error(res).ErrorMessage());
			return false;
		}

		m_d3d12Debug->EnableDebugLayer();
	}


	return true;
}

void DXDebugLayer::setD3DDebug(SDL_Renderer& renderer)
{
	if (!isEnabled())
	{
		return;
	}

	{
		if (ID3D11Device* device = SDL_RenderGetD3D11Device(&renderer))
		{
			HRESULT res = device->QueryInterface(IID_PPV_ARGS(m_d3d11Debug.GetAddressOf()));;
			device->Release();

			if (!SUCCEEDED(res))
			{
				CZ_LOG(Warning, "Could not get D3D11Debug interface. Error={}.", _com_error(res).ErrorMessage());
			}

			return;
		}
	}

	{
		if (ID3D12Device* device = SDL_RenderGetD3D12Device(&renderer))
		{
			HRESULT res = device->QueryInterface(IID_PPV_ARGS(&m_d3d12DebugDevice));
			device->Release();

			if (!SUCCEEDED(res))
			{
				CZ_LOG(Warning, "Could not get D3D12Debug interface. Error={}.", _com_error(res).ErrorMessage());
			}

			return;
		}
	}

	CZ_LOG(Warning, "No compatible SDL_Renderer found for the DXDebugLayer");
}

void DXDebugLayer::shutdown()
{
	if (!isEnabled())
	{
		return;
	}

	m_dxgiInfoQueue = nullptr;

	if (m_d3d11Debug)
	{
		OutputDebugStringA("MLGE: ID3D11Debug::ReportLiveDeviceObjects start.\n");
		m_d3d11Debug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY | D3D11_RLDO_DETAIL | D3D11_RLDO_IGNORE_INTERNAL);
		OutputDebugStringA("MLGE: ID3D11Debug::ReportLiveDeviceObjects end. If only ID3D11Device was reported as alive (~1..2 refs), then it's fine.\n");

	}

	if (m_d3d12DebugDevice)
	{
		OutputDebugStringA("MLGE: ID3D12DebugDevice::ReportLiveDeviceObjects start.\n");
		m_d3d12DebugDevice->ReportLiveDeviceObjects(D3D12_RLDO_SUMMARY | D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
		OutputDebugStringA("MLGE: ID3D12DebugDevice::ReportLiveDeviceObjects end. If only ID3D12Device was reported as alive (~1..2 refs), then it's fine.\n");
	}

	// Release, so m_dxdgiDebug doesn't see this as a leak
	m_d3d11Debug = nullptr;
	m_d3d12Debug = nullptr;
	m_d3d12DebugDevice = nullptr;

	if (m_dxgiDebug)
	{
		OutputDebugStringA("MLGE: IDXGIDebug::ReportLiveObjects start...\n");
		m_dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
		OutputDebugStringA("MLGE: IDXGIDebug::ReportLiveObjects end. If this reported any warnings, then very likely there are leaks\n");
		m_dxgiDebug = nullptr;
	}

}

} // namespace mlge

