#if MLGE_EDITOR

#include "mlge/Editor/AssetBrowser.h"
#include "mlge/Editor/Editor.h"

namespace mlge::editor
{

AssetBrowser::AssetBrowser(bool& p_open)
	: Window(p_open, "Asset Browser")
{
	m_flags = ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody;

	refreshList();

	m_assetTypeRoot.ptr = &Class::get<MResourceDefinition>();
	buildAssetTypeTree(m_assetTypeRoot);
}

void AssetBrowser::buildAssetTypeTree(AssetTypeNode& node)
{
	for(auto it = Class::begin(); it!= Class::end(); ++it)
	{
		Class* c = *it;
		if (c->getParentClass() == node.ptr)
		{
			node.derived.push_back(std::make_unique<AssetTypeNode>());
			node.derived.back()->ptr = c;
			node.derived.back()->name = std::string(c->getName(), c->getName() + strlen(c->getName()) - static_cast<int>(strlen("Definition")));
			buildAssetTypeTree(*node.derived.back());
		}
	}
}

void AssetBrowser::refreshList()
{
	m_rootNode.children.clear();

	std::vector<ObjectPtr<MResourceDefinition>> defs = ResourceManager::get().getAllDefinitions();
	std::sort(defs.begin(), defs.end(),
		[](ObjectPtr<MResourceDefinition>& a, ObjectPtr<MResourceDefinition>& b)
	{
		return a->name < b->name;
	});


	auto getNameToken = [](const char*& ptr) -> std::string_view
	{
		while(*ptr == '/')
		{
			ptr++;
		}

		const char* start = ptr;
		while (*ptr != '/' && *ptr != 0)
		{
			ptr++;
		}
		if (*ptr == '/') ptr++;
		return std::string_view(start, ptr);
	};

	for(auto&& def : defs)
	{
		const char* ptr = def->name.c_str();

		AssetNode* node = &m_rootNode;
		while(*ptr != 0)
		{
			std::string_view token = getNameToken(ptr);

			auto newNode = node->tryGet(token);
			if (newNode)
			{
				node = newNode;
			}
			else
			{
				node->children.push_back(std::make_unique<AssetNode>());
				node = node->children.back().get();
				node->name = token;
			}
		}

		node->def = def;
		node->fullname = def->name;
	}

}

AssetBrowser::~AssetBrowser()
{
}

void AssetBrowser::displayAssetNode(AssetNode& node)
{
	const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_SpanAllColumns;
	ImGui::TableNextRow();
	ImGui::TableNextColumn();
	const bool isFolder = node.def ? false : true;
	if (isFolder)
	{
		bool open = ImGui::TreeNodeEx(node.name.c_str(), treeNodeFlags);
		ImGui::TableNextColumn();
		ImGui::TextUnformatted("");
		if (open)
		{
			for(auto& child : node.children)
			{
				displayAssetNode(*child);
			}
			ImGui::TreePop();
		}
	}
	else
	{
		ImGui::TreeNodeEx(node.name.c_str(), treeNodeFlags | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen);
		if (ImGui::IsMouseDoubleClicked(0) && ImGui::IsItemHovered())
		{
			// If there is already a Resource Window opened for this, then don't do anything
			if (Editor::get().findWindowByTag(node.def.get()))
			{
				return;
			}

			std::unique_ptr<Window> w = node.def->createEditWindow();
			w->setTag(node.def.get());
			Editor::get().addWindow(std::move(w));
		}

		ImGui::TableNextColumn();
		ImGui::TextUnformatted(std::string(node.def->getTypeName()).c_str());
	}
}

void AssetBrowser::displayAssetRootNode()
{
	for (auto& child : m_rootNode.children)
	{
		displayAssetNode(*child);
	}
}

void AssetBrowser::displayNewAssetTypeNode(AssetTypeNode& node)
{
	auto doItem = [](AssetTypeNode& node)
	{
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay))
		{
			if (node.ptr->isAbstract())
			{
				ImGui::SetTooltip((std::string("(Abstract)\n") + node.ptr->getDescription()).c_str());
			}
			else
			{
				ImGui::SetTooltip(node.ptr->getDescription());
			}
		}

		if (ImGui::IsItemClicked() && !node.ptr->isAbstract())
		{
			if (Editor::get().findWindowByTag(&node))
			{
				return;
			}

			ObjectPtr<MResourceDefinition> def = static_pointer_cast<MResourceDefinition>(node.ptr->createObject());
			std::unique_ptr<Window> w =def->createEditWindow();
			w->setTag(&node);
			Editor::get().addWindow(std::move(w));
		}
	};

	for(auto&& child : node.derived)
	{
		if (child->derived.size())
		{
			if (ImGui::BeginMenu(child->name.c_str()))
			{
				doItem(*child);
				displayNewAssetTypeNode(*child);
				ImGui::EndMenu();
			}
		}
		else
		{
			ImGui::MenuItem(child->name.c_str());
			doItem(*child);
		}
	}
}

void AssetBrowser::show()
{
	if (ImGui::Button("Reload"))
	{
		refreshList();
	}

	ImGui::SameLine();
	if (ImGui::Button("New"))
	{
		ImGui::OpenPopup("New_Popup");
	}

	if (ImGui::BeginPopup("New_Popup"))
	{
		displayNewAssetTypeNode(m_assetTypeRoot);
		ImGui::EndPopup();
	}

	const float TEXT_BASE_WIDTH = ImGui::CalcTextSize("A").x;

	if (ImGui::BeginTable("test", 2, m_flags))
	{
		ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_NoHide);
		ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 20.0f);
		ImGui::TableHeadersRow();

		displayAssetRootNode();

		ImGui::EndTable();
	}
}

} // namespace mlge::editor

#endif

