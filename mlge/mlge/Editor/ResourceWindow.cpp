
#if MLGE_EDITOR

#include "mlge/Editor/ResourceWindow.h"
#include "mlge/Resource/Resource.h"

#include "crazygaze/core/StringUtils.h"

namespace mlge::editor
{

namespace
{
	std::string constructWindowTitle(MResourceDefinition& target)
	{
		return std::format("{} Editor - {}", target.getTypeName(), target.name == "" ? "(New)" : target.name);
	}
}


BaseResourceWindow::BaseResourceWindow(MResourceDefinition& target)
	: Window(m_this_p_open, constructWindowTitle(target))
	, m_target(&target)
{
}

BaseResourceWindow::~BaseResourceWindow()
{
}

bool BaseResourceWindow::showFilePath(const char* title, fs::path& path, const std::string& extensions, std::string rootFolder)
{
	if (showString(title, path))
	{
		return true;
	}

	ImGui::SameLine();

	auto prevChangeCount = m_changedCount;
	if (ImGui::Button("Browse"))
	{
		if (rootFolder == "")
		{
			rootFolder = narrow((getGamePath() / "Assets" / path).c_str());
		}

		ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", title, extensions.c_str(), rootFolder.c_str(), 1, nullptr, ImGuiFileDialogFlags_Modal);
	}

	if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
	{
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
			std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
			fs::path rel = fs::relative(filePathName, getGamePath() / "Assets" / "");

			if (rel != path)
			{
				path = rel;
				setDirty();
			}
		}

		ImGuiFileDialog::Instance()->Close();
	}

	return prevChangeCount != m_changedCount;
}

bool BaseResourceWindow::showString(const char* title, std::string& str)
{
	if (ImGui::InputText(title, &str))
	{
		setDirty();
		return true;
	}
	else
	{
		return false;
	}
}

bool BaseResourceWindow::showString(const char* title, fs::path& path)
{
	std::string str = narrow(path.c_str());
	if (ImGui::InputText(title, &str))
	{
		fs::path newPath = str;
		if (newPath != path)
		{
			path = newPath;
			setDirty();
			return true;
		}
	}

	return false;
}

void BaseResourceWindow::showGenericButtons()
{
	if (m_dirty)
	{
		if (ImGui::Button("Save"))
		{
			if (save())
			{
				*m_p_open = true;
			}
		}
	}
}

void BaseResourceWindow::setDirty()
{
	m_dirty = true;
	m_changedCount++;
}

bool BaseResourceWindow::tick(float elapsedSeconds)
{
	int prevChangedCount = m_changedCount;

	ImGui::SetNextWindowSize(ImVec2(640,480), ImGuiCond_FirstUseEver);

	if (!Super::tick(elapsedSeconds))
	{
		return false;
	}

	if (m_dirty && (m_changedCount != prevChangedCount))
	{
		m_resource = nullptr;

		if (m_target->isLoaded())
		{
			CZ_LOG(Warning, "Resource still loaded. Potential bug in {}", m_title);
		}

		reloadResource();
	}

	return true;
}

} // namespace mlge::editor

#endif

