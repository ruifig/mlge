#pragma once

#if MLGE_EDITOR

#include "mlge/Editor/Window.h"
#include "mlge/Resource/Resource.h"

#include "crazygaze/core/Singleton.h"

namespace mlge::editor
{

/**
 * Editor window to list resources
 */
class AssetBrowser : public Window, public Singleton<AssetBrowser>
{
  public:
	AssetBrowser(bool& p_open);
	~AssetBrowser();
	using Super = Window;

  protected:

	virtual void show() override;
	void refreshList();

	struct AssetNode
	{
		std::string name;
		std::string fullname;

		// If not set, the node is a folder.
		ObjectPtr<MResourceDefinition> def;

		std::vector<std::unique_ptr<AssetNode>> children;

		AssetNode* tryGet(std::string_view nameToFind)
		{
			for(auto& child : children)
			{
				if (child->name == nameToFind)
				{
					return child.get();
				}
			}

			return nullptr;
		}

	};

	AssetNode m_rootNode;
	void displayAssetNode(AssetNode& node);
	void displayAssetRootNode();

	struct AssetTypeNode
	{
		Class* ptr;
		std::string name;
		std::vector<std::unique_ptr<AssetTypeNode>> derived;
	};
	AssetTypeNode m_assetTypeRoot;

	void buildAssetTypeTree(AssetTypeNode& node);
	void displayNewAssetTypeNode(AssetTypeNode& node);

	ImGuiTableFlags m_flags;
};


} // namespace mlge

#endif

