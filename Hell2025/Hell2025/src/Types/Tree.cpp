#include "Tree.h"
#include "AssetManagement/AssetManager.h"
#include "Util.h"

Tree::Tree(TreeCreateInfo createInfo) {
    m_transform.position = createInfo.position;
    m_transform.rotation = createInfo.rotation;
    m_transform.scale = createInfo.scale;
    m_treeType = (TreeType)createInfo.type;

    if (m_treeType == TreeType::TREE_LARGE_0) {
        m_model = AssetManager::GetModelByName("TreeLarge_0");
        m_material = AssetManager::GetMaterialByName("TreeLarge_0");
    }
    else if (m_treeType == TreeType::TREE_LARGE_1) {
        m_model = AssetManager::GetModelByName("TreeLarge_1");
        m_material = AssetManager::GetMaterialByName("TreeLarge_0");
    }
    else if (m_treeType == TreeType::TREE_LARGE_2) {
        m_model = AssetManager::GetModelByName("TreeLarge_2");
        m_material = AssetManager::GetMaterialByName("TreeLarge_0");
    }
}

TreeCreateInfo Tree::GetCreateInfo() {
    TreeCreateInfo createInfo;
    createInfo.position = m_transform.position;
    createInfo.rotation = m_transform.rotation;
    createInfo.scale = m_transform.scale;
    createInfo.type = (int)(m_treeType);
    return createInfo;
}

void Tree::BeginFrame() {
    m_isSelected = false;
}

void Tree::MarkAsSelected() {
    m_isSelected = true;
}

bool Tree::IsSelected() {
    return m_isSelected;
}

void Tree::Update(float deltaTime) {
    m_modelMatrix = m_transform.to_mat4();
    UpdateRenderItems();
}

void Tree::SetMousePickIndex(int mousePickIndex) {
    m_mousePickIndex = mousePickIndex;
}

void Tree::SetPosition(glm::vec3 position) {
    m_transform.position = position;
}

void Tree::UpdateRenderItems() {
    m_renderItems.clear();

    for (int i = 0; i < m_model->GetMeshCount(); i++) {
        Mesh* mesh = AssetManager::GetMeshByIndex(m_model->GetMeshIndices()[i]);
        if (mesh) {
            RenderItem renderItem;
            renderItem.mousePickType = (int)ObjectType::TREE;
            renderItem.mousePickIndex = m_mousePickIndex;
            renderItem.modelMatrix = GetModelMatrix();
            renderItem.inverseModelMatrix = glm::inverse(renderItem.modelMatrix);
            renderItem.meshIndex = m_model->GetMeshIndices()[i];
            renderItem.baseColorTextureIndex = m_material->m_basecolor;
            renderItem.normalMapTextureIndex = m_material->m_normal;
            renderItem.rmaTextureIndex = m_material->m_rma;
            m_renderItems.push_back(renderItem);
        }
    }
}
