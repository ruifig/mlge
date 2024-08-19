#pragma once

#if MLGE_EDITOR

#include "mlge/Editor/Window.h"
#include "mlge/Object.h"
#include "mlge/Paths.h"

namespace mlge
{
	class MResourceDefinition;
	class MResource;
}

namespace mlge::editor
{

class BaseResourceWindow : public Window
{
  public:
	using Super = Window;

	BaseResourceWindow(MResourceDefinition& target);
	virtual ~BaseResourceWindow();

  protected:

	void setDirty();

	ObjectPtr<MResourceDefinition> m_target;
	// Resource, if loaded
	ObjectPtr<MResource> m_resource;

	bool m_this_p_open = true;
	bool m_dirty = false;
	int m_changedCount = 0;

	bool showFilePath(const char* title, fs::path& path, const std::string& extensions, std::string rootFolder);
	bool showString(const char* title, std::string& path);
	bool showString(const char* title, fs::path& str);

	void showGenericButtons();

	virtual bool tick(float elapsedSeconds);

	virtual void reloadResource()
	{
	}

	virtual bool save()
	{
		return true;
	}
};

template<typename ResourceDefinitionType, typename ResourceType>
class ResourceWindow : public BaseResourceWindow
{
  public:
	using Super = BaseResourceWindow;

	ResourceWindow(ResourceDefinitionType& target)
		: BaseResourceWindow(target)
	{
	}

	ResourceDefinitionType& getDef()
	{
		return *static_cast<ResourceDefinitionType*>(m_target.get());
	}

  protected:
		ObjectPtr<ResourceType> m_resource;
};

} // namespace mlge::editor

#endif

