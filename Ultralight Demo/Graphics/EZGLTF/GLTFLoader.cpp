#include "PCH.h"
#include "GLTFLoader.h"
#include "TinyGLTFEnumToString.h"

using namespace EZGLTF;

bool GLTFLoader::LoadGLTFFile(std::string inFilePath, ParsedGLTFModel& outModel)
{
    m_FilePath = inFilePath;
    m_ParseModel = &outModel;
    m_ParseModel->Reset(); //This should always be called with a fresh ParseGLTFModel instance, but just in case...
    m_ErrorMessage = "";

    m_BaseDirectory = DirectoryHelper::GetDirectoryFromPath(inFilePath);

    std::ifstream inFile(inFilePath, std::ios::binary | std::ios::in | std::ios::ate);
    if (!inFile.is_open())
    {
        SetErrorMessage(strfmt("Failed to open file [%s].", inFilePath.c_str()));
        return false;
    }
    std::streampos fileSize = inFile.tellg();
    inFile.seekg(0, std::ios::beg);

    std::unique_ptr<char[]> pData = std::make_unique<char[]>(fileSize);
    if (pData == nullptr)
    {
        SetErrorMessage(strfmt("Failed to allocate block of %d bytes to store GLTF ascii data.", fileSize));
        return false;
    }

    inFile.read(pData.get(), fileSize);
    if (inFile.fail())
    {
        inFile.close();
        SetErrorMessage(strfmt("Filestream was corrupted while reading bytes for gltf ascii data from file [%s].", inFilePath.c_str()));
        return false;
    }
    inFile.close();

    tinygltf::TinyGLTF gltf_context;
    std::string gltf_error = "";
    std::string gltf_warning = "";
    unsigned int dataBytes = fileSize;
    bool result = gltf_context.LoadASCIIFromString(&m_GLTFModel,
                                                   &gltf_error,
                                                   &gltf_warning,
                                                   pData.get(),
                                                   dataBytes,
                                                   m_BaseDirectory);
    if (result == false)
    {
        SetErrorMessage(strfmt("Failed to load gltf file. Tiny GLTF error [%s].", gltf_error.c_str()));
        return false;
    }

    if (!ProcessModel())
    {
        return false;
    }

    return true;
}

string GLTFLoader::GetErrorMessage()
{
    return m_ErrorMessage;
}

bool GLTFLoader::ProcessModel()
{
    auto& gltfModel = m_GLTFModel;
    for (auto& accessor : gltfModel.accessors)
    {
        if (accessor.sparse.isSparse == true)
        {
            SetErrorMessage("Model contains sparse accessors which are not supported.");
            return false;
        }
    }

    for (auto& mesh : gltfModel.meshes)
    {
        for (auto& weight : mesh.weights)
        {
            if (weight != 0)
            {
                SetErrorMessage("Mesh having a default weight on a morph target is not supported. Fix your model.");
                return false;
            }
        }
    }

    if (!ProcessNodes())
    {
        return false;
    }

    if (!ProcessMaterials())
    {
        return false;
    }

    if (!ProcessMeshes())
    {
        return false;
    }

    if (!ProcessAnimations())
    {
        return false;
    }

    //RemoveUnusedMeshesAndMaterials();

    //for (auto& node : m_ParseModel->Nodes)
    //{
    //    if (node.BoneId != -1)
    //    {
    //        m_ParseModel->BoneCount += 1;
    //    }
    //}

    ////Validate if this is a model that uses LODs and if so validate that all of the nodes are properly named
    //if (!CheckForLODs())
    //{
    //    return false;
    //}

    //if (!BuildLODSets())
    //{
    //    return false;
    //}

    BuildGlobalSpacePositionsAndIndices();

    //Build AABB
    for (auto& vert : m_ParseModel->PositionsInGlobalSpace)
    {
        m_ParseModel->AABB.ProcessCoord(vert);
    }

    for (auto& mesh : m_ParseModel->Meshes)
    {
        for (auto& prim : mesh.Primitives)
        {
            if (prim.Positions.size() == 0)
            {
                SetErrorMessage(strfmt("One of the primitives for mesh [%s] has no position data.", mesh.Name.c_str()));
                return false;
            }
            if (prim.Normals.size() > 0)
            {
                if (prim.Normals.size() != prim.Positions.size())
                {
                    SetErrorMessage(strfmt("Normals vertex count does not match position vertex count for primitive on mesh [%s].", mesh.Name.c_str()));
                    return false;
                }
            }
            if (prim.TextureCoordinates.size() > 0)
            {
                if (prim.TextureCoordinates.size() != prim.Positions.size())
                {
                    SetErrorMessage(strfmt("Texcoords vertex count does not match position vertex count for primitive on mesh [%s].", mesh.Name.c_str()));
                    return false;
                }
            }
            if (prim.Colors.size() > 0)
            {
                if (prim.Colors.size() != prim.Positions.size())
                {
                    SetErrorMessage(strfmt("Color vertex count does not match position vertex count for primitive on mesh [%s].", mesh.Name.c_str()));
                    return false;
                }
            }
            if (prim.JointIndices.size() > 0)
            {
                if (prim.JointIndices.size() != prim.Positions.size())
                {
                    SetErrorMessage(strfmt("Joint indices group count does not match position vertex count for primitive on mesh [%s].", mesh.Name.c_str()));
                    return false;
                }
            }
            if (prim.JointIndices.size() != prim.JointWeights.size())
            {
                if (prim.JointIndices.size() != prim.Positions.size())
                {
                    SetErrorMessage(strfmt("Joint index and joint weight mismatch for primitive on mesh [%s].", mesh.Name.c_str()));
                    return false;
                }
            }
        }
    }

    return true;
}

bool EZGLTF::GLTFLoader::ProcessNodes()
{
    auto& gltfModel = m_GLTFModel;

    if (gltfModel.nodes.size() == 0)
    {
        SetErrorMessage("This model has no nodes. That is not valid.");
        return false;
    }

    if (gltfModel.skins.size() > 1)
    {
        SetErrorMessage(strfmt("This model has %d skins. Currently only up to 1 skin is supported.", gltfModel.skins.size()));
        return false;
    }

    m_ParseModel->Nodes.resize(gltfModel.nodes.size());

    //Build each node's name/transform/identify the skeleton global node
    int newNodeIndex = 0;
    std::vector<int> potentialRootNode;

    for (int i = 0; i < gltfModel.nodes.size(); i++)
    {
        auto& gltfNode = gltfModel.nodes[i];
        auto& mw3dNode = m_ParseModel->Nodes[i];
        mw3dNode.Name = gltfNode.name;

        if (gltfNode.mesh != -1)
        {
            mw3dNode.MeshIndices.push_back(gltfNode.mesh);
        }

        if (gltfNode.mesh != -1 &&
            gltfNode.skin != -1)
        {
            //TODO: Need to review this whole thing... Identifying the global node has always been confusing.
            m_ParseModel->SkeletonGlobalNode = i;
            mw3dNode.SkinId = gltfNode.skin;
            potentialRootNode.push_back(i);
        }

        if (gltfNode.matrix.size() > 0) //A node can either have a matrix or have specified SRT (scale, rotation, translation)
        {
            if (gltfNode.matrix.size() != 16)
            {
                SetErrorMessage("One of the nodes has a matrix that is not 16 values. This shouldn't be possible.");
                return false;
            }

            float data[16];
            for (int i = 0; i < 16; i++)
            {
                data[i] = gltfNode.matrix[i]; //Matrix is in doubles only God knows why
            }

            DirectX::XMFLOAT4X4 mat_f4x4(data);
            DirectX::XMMATRIX mat = DirectX::XMLoadFloat4x4(&mat_f4x4);

            DirectX::XMVECTOR scaleVec;
            DirectX::XMVECTOR translationVec;
            DirectX::XMVECTOR rotationVec;

            DirectX::XMMatrixDecompose(&scaleVec, &rotationVec, &translationVec, mat);

            DirectX::XMFLOAT3 scale;
            DirectX::XMStoreFloat3(&scale, scaleVec);
            DirectX::XMFLOAT3 translation;
            DirectX::XMStoreFloat3(&translation, translationVec);
            DirectX::XMFLOAT4 rotation;
            DirectX::XMStoreFloat4(&rotation, rotationVec);
            rotation.x = rotation.x;
            rotation.y = rotation.y;
            rotation.z = rotation.z;
            rotation.w = rotation.w;

            mw3dNode.Scale = scale;
            mw3dNode.QuaternionRotation = rotation;
            mw3dNode.Translation = translation;
        }
        else
        {
            if (gltfNode.scale.size() > 0)
            {
                mw3dNode.Scale.x = gltfNode.scale[0];
                mw3dNode.Scale.y = gltfNode.scale[1];
                mw3dNode.Scale.z = gltfNode.scale[2];
            }
            if (gltfNode.rotation.size() > 0)
            {
                mw3dNode.QuaternionRotation.x = gltfNode.rotation[0];
                mw3dNode.QuaternionRotation.y = gltfNode.rotation[1];
                mw3dNode.QuaternionRotation.z = gltfNode.rotation[2];
                mw3dNode.QuaternionRotation.w = gltfNode.rotation[3];
            }
            if (gltfNode.translation.size() > 0)
            {
                mw3dNode.Translation.x = gltfNode.translation[0];
                mw3dNode.Translation.y = gltfNode.translation[1];
                mw3dNode.Translation.z = gltfNode.translation[2];
            }
        }

        tinygltf::Node* pGltfNode = &gltfModel.nodes[i];
        for (auto childIndex : pGltfNode->children)
        {
            m_ParseModel->Nodes[childIndex].ParentIndex = i;
            mw3dNode.ChildrenIndices.push_back(childIndex);
        }
    }

    for (auto& node : m_ParseModel->Nodes)
    {
        DirectX::XMVECTOR quatRotVec = DirectX::XMLoadFloat4(&node.QuaternionRotation);
        node.DefaultLocalTransform = DirectX::XMMatrixScaling(node.Scale.x, node.Scale.y, node.Scale.z) *
            DirectX::XMMatrixRotationQuaternion(quatRotVec) *
            DirectX::XMMatrixTranslation(node.Translation.x, node.Translation.y, node.Translation.z);
    }

    //If this model has a skin, we need to assign the Bone ID's to each node
    if (gltfModel.skins.size() == 1)
    {
        auto& skin = gltfModel.skins[0];
        if (skin.inverseBindMatrices == -1)
        {
            SetErrorMessage("This model has a skin, but is missing inverse bind matrix data for the joints. This is not valid.");
            return false;
        }

        for (int i = 0; i < skin.joints.size(); i++)
        {
            const int nodeIndex = skin.joints[i];
            m_ParseModel->Nodes[nodeIndex].BoneId = i;
        }

        auto& matrixAccessor = gltfModel.accessors[skin.inverseBindMatrices];
        auto& matrixBufferView = gltfModel.bufferViews[matrixAccessor.bufferView];
        auto& matrixBuffer = gltfModel.buffers[matrixBufferView.buffer];

        if (matrixAccessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT)
        {
            SetErrorMessage(strfmt("The skin for this model has an invalid matrix accessor component type. It should be of type TINYGLTF_COMPONENT_TYPE_FLOAT. It is [%s].", TinyGLTFComponentTypeToString(matrixAccessor.componentType)).c_str());
            return false;
        }

        if (matrixAccessor.type != TINYGLTF_TYPE_MAT4)
        {
            //TODO Add a way to get the string value for these defines
            SetErrorMessage(strfmt("The skin for this model has an invalid matrix accessor type. It should be of type TINYGLTF_TYPE_MAT4. It is [%s].", TinyGLTFTypeToString(matrixAccessor.type)).c_str());
            return false;
        }

        void* matrixEntries = &matrixBuffer.data[matrixBufferView.byteOffset + matrixAccessor.byteOffset];
        for (int i = 0; i < matrixAccessor.count; i++)
        {
            void* matrixEntryPtr = nullptr;
            if (matrixBufferView.byteStride == 0)
            {
                matrixEntryPtr = &matrixBuffer.data[matrixBufferView.byteOffset + matrixAccessor.byteOffset + i * sizeof(Matrix)];
            }
            else
            {
                matrixEntryPtr = &matrixBuffer.data[matrixBufferView.byteOffset + matrixAccessor.byteOffset + matrixBufferView.byteStride * i];
            }
            Matrix* matrixEntry = (Matrix*)matrixEntryPtr;
            int nodeIndex = skin.joints[i];
            m_ParseModel->Nodes[nodeIndex].InverseBindTransform = *matrixEntry;
        }
    }

    //I want to sort the nodes, because they're in some nonsensical order. It makes sense to sort them from roots to tips for building animations/skeleton.
    //Key = Old Index
    //Value = New Index
    //Ex. lookup_OriginalSortedNodes[0] = 1  would mean that the node originally at index 0 is now at index 1
    std::map<int, int> lookup_OriginalToSortedNodes;

    std::function<void(std::map<int, int>& mapToStoreLookups,
                       std::vector<SkeletonNode>& nodesVector,
                       int currentNodeIndex)> MapNode;

    MapNode = [&](std::map<int, int>& mapToStoreLookups,
                  std::vector<SkeletonNode>& nodesVector,
                  int currentNodeIndex)
        {
            auto& node = nodesVector[currentNodeIndex];
            mapToStoreLookups[currentNodeIndex] = mapToStoreLookups.size();
            for (int i = 0; i < node.ChildrenIndices.size(); i++)
            {
                int childIndex = node.ChildrenIndices[i];
                MapNode(mapToStoreLookups,
                        nodesVector,
                        childIndex);
            }
        };

    for (int i = 0; i < m_ParseModel->Nodes.size(); i++)
    {
        auto& node = m_ParseModel->Nodes[i];
        if (node.ParentIndex == -1) //Root node - we start at root and this is recursively called to make sure the reordering makes sense for root->tip
        {
            MapNode(lookup_OriginalToSortedNodes, m_ParseModel->Nodes, i);
        }
    }
    //Now our lookup map is set up. Need to rebuild nodes now in correct sorted order
    std::vector<SkeletonNode> oldNodesVector = m_ParseModel->Nodes; //copy to a temp vector to reference when rebuilding with sorted nodes
    m_ParseModel->Nodes.clear();
    m_ParseModel->Nodes.resize(oldNodesVector.size());
    for (auto& lookupData : lookup_OriginalToSortedNodes)
    {
        int oldIndex = lookupData.first;
        int newIndex = lookupData.second;
        m_ParseModel->Nodes[newIndex] = oldNodesVector[oldIndex];
    }

    //Update our reverse map lookup
    m_LookupOldNodeToNewNodeIndex = lookup_OriginalToSortedNodes;
    for (auto& oldToNew : m_LookupOldNodeToNewNodeIndex)
    {
        m_LookupNewNodeToOldNodeIndex[oldToNew.second] = oldToNew.first;
    }

    //Update the global skeleton node
    if (m_ParseModel->SkeletonGlobalNode != -1)
        m_ParseModel->SkeletonGlobalNode = m_LookupOldNodeToNewNodeIndex[m_ParseModel->SkeletonGlobalNode];

    for (int i = 0; i < m_ParseModel->Nodes.size(); i++)
    {
        auto& node = m_ParseModel->Nodes[i];
        for (auto& childIndex : node.ChildrenIndices)
        {
            childIndex = m_LookupOldNodeToNewNodeIndex[childIndex];
        }
        if (node.ParentIndex != -1)
        {
            node.ParentIndex = m_LookupOldNodeToNewNodeIndex[node.ParentIndex];
        }
    }

    return true;
}

bool EZGLTF::GLTFLoader::ProcessMaterials()
{
    auto& gltfModel = m_GLTFModel;
    m_ParseModel->Materials.resize(gltfModel.materials.size());
    for (int i = 0; i < gltfModel.materials.size(); i++)
    {
        auto& gltfMaterial = gltfModel.materials[i];
        auto& mat = m_ParseModel->Materials[i];
        mat = make_shared<Material>();

        mat->Name = gltfMaterial.name;
        mat->Properties.BaseColor.x = gltfMaterial.pbrMetallicRoughness.baseColorFactor[0];
        mat->Properties.BaseColor.y = gltfMaterial.pbrMetallicRoughness.baseColorFactor[1];
        mat->Properties.BaseColor.z = gltfMaterial.pbrMetallicRoughness.baseColorFactor[2];
        mat->Properties.BaseColor.w = gltfMaterial.pbrMetallicRoughness.baseColorFactor[3];
        mat->Properties.MetallicFactor = gltfMaterial.pbrMetallicRoughness.metallicFactor;
        mat->Properties.RoughnessFactor = gltfMaterial.pbrMetallicRoughness.roughnessFactor;

        if (gltfMaterial.pbrMetallicRoughness.baseColorTexture.index != -1) //If there is a texture for the base color...
        {
            auto& texture = gltfModel.textures[gltfMaterial.pbrMetallicRoughness.baseColorTexture.index];
            auto& img = gltfModel.images[texture.source];
            std::string textureName = "";
            //Replace %20 with space
            for (int i = 0; i < img.uri.size(); i++) //this is terrible -- why is replacing substring so bad in c++?
            {
                if (img.uri[i] == '%')
                {
                    if (img.uri[i + 1] == '2')
                    {
                        if (img.uri[i + 2] == '0')
                        {
                            textureName += " ";
                            i += 2;
                            continue;
                        }
                    }
                }
                textureName += img.uri[i];
            }
            mat->BaseColorTexture = make_shared<Texture>();
            bool success = mat->BaseColorTexture->Initialize(m_BaseDirectory + "/" + textureName);
            if (success == false)
            {
                m_ErrorMessage = strfmt("Failed to load texture [%s].", textureName.c_str());
                return false;
            }
        }
    }
    return true;
}

bool EZGLTF::GLTFLoader::ProcessMeshes()
{
    auto& gltfModel = m_GLTFModel;

    m_ParseModel->Meshes.resize(gltfModel.meshes.size());
    for (int i = 0; i < gltfModel.meshes.size(); i++)
    {
        auto& gltfMesh = gltfModel.meshes[i];
        auto& mw3dMesh = m_ParseModel->Meshes[i];

        mw3dMesh.Name = gltfMesh.name;
        mw3dMesh.Primitives.resize(gltfMesh.primitives.size());

        for (int j = 0; j < gltfMesh.primitives.size(); j++)
        {
            auto& gltfPrim = gltfMesh.primitives[j];
            auto& mw3dPrim = mw3dMesh.Primitives[j];

            if (gltfPrim.mode != TINYGLTF_MODE_TRIANGLES)
            {
                SetErrorMessage(strfmt("Expected primitive mode of TINYGLTF_MODE_TRIANGLES. Actual primitive mode is [%s]",
                                     TinyGLTFModeToString(gltfPrim.mode).c_str()));
                return false;
            }

            if (gltfPrim.material >= 0)
            {
                mw3dPrim.Material = m_ParseModel->Materials[gltfPrim.material];
            }

            for (auto& [attribute, accessorIndex] : gltfPrim.attributes)
            {
                auto& accessor = gltfModel.accessors[accessorIndex];
                auto& bufferView = gltfModel.bufferViews[accessor.bufferView];
                auto& buffer = gltfModel.buffers[bufferView.buffer];

                int startingOffset = bufferView.byteOffset + accessor.byteOffset;

                if (attribute == "POSITION" || attribute == "NORMAL")
                {
                    if (accessor.type != TINYGLTF_TYPE_VEC3)
                    {
                        SetErrorMessage(strfmt("Mesh attribute processing error. [%s] accessor expected to be type TINYGLTF_TYPE_VEC3. Actual type is [%s].",
                                             attribute,
                                             TinyGLTFTypeToString(accessor.type).c_str()));
                        return false;
                    }
                    if (accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT)
                    {
                        SetErrorMessage(strfmt("Mesh attribute processing error. [%s] accessor expected to be component type TINYGLTF_COMPONENT_TYPE_FLOAT. Actual type is [%s].",
                                             attribute,
                                             TinyGLTFComponentTypeToString(accessor.componentType).c_str()));
                        return false;
                    }
                    if (attribute == "POSITION")
                    {
                        mw3dPrim.Positions.resize(accessor.count);
                        if (bufferView.byteStride == 0)
                        {
                            memcpy(&mw3dPrim.Positions[0],
                                   &buffer.data[startingOffset],
                                   accessor.count * sizeof(DirectX::XMFLOAT3));
                        }
                        else //Interleaved data
                        {
                            for (int k = 0; k < accessor.count; k++)
                            {
                                void* pData = &buffer.data[startingOffset + bufferView.byteStride * k];
                                memcpy(&mw3dPrim.Positions[k],
                                       pData,
                                       sizeof(DirectX::XMFLOAT3));
                            }
                        }
                    }
                    if (attribute == "NORMAL")
                    {
                        mw3dPrim.Normals.resize(accessor.count);
                        if (bufferView.byteStride == 0)
                        {
                            memcpy(&mw3dPrim.Normals[0],
                                   &buffer.data[startingOffset],
                                   accessor.count * sizeof(DirectX::XMFLOAT3));
                        }
                        else //Interleaved data
                        {
                            for (int i = 0; i < accessor.count; i++)
                            {
                                void* pData = &buffer.data[startingOffset + bufferView.byteStride * i];
                                memcpy(&mw3dPrim.Normals[i],
                                       pData,
                                       sizeof(DirectX::XMFLOAT3));
                            }
                        }
                    }
                }
                if (attribute == "TEXCOORD_0")
                {
                    if (accessor.type != TINYGLTF_TYPE_VEC2)
                    {
                        SetErrorMessage(strfmt("Mesh attribute processing error. TEXCOORD accessor expected to be type TINYGLTF_TYPE_VEC2. Actual type is [%s].",
                                             TinyGLTFTypeToString(accessor.componentType).c_str()));
                        return false;
                    }
                    if (accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT)
                    {
                        SetErrorMessage(strfmt("Mesh attribute processing error. TEXCOORD accessor expected to be component type TINYGLTF_COMPONENT_TYPE_FLOAT. Actual type is [%s].",
                                             TinyGLTFComponentTypeToString(accessor.componentType).c_str()));
                        return false;
                    }
                    mw3dPrim.TextureCoordinates.resize(accessor.count);

                    if (bufferView.byteStride == 0)
                    {
                        memcpy(&mw3dPrim.TextureCoordinates[0],
                               &buffer.data[startingOffset],
                               accessor.count * sizeof(DirectX::XMFLOAT2));
                    }
                    else
                    {
                        for (int k = 0; k < accessor.count; k++)
                        {
                            void* pData = &buffer.data[startingOffset + bufferView.byteStride * k];
                            memcpy(&mw3dPrim.TextureCoordinates[k],
                                   pData,
                                   sizeof(DirectX::XMFLOAT2));
                        }
                    }

                }
                if (attribute == "COLOR_0")
                {
                    if (accessor.type != TINYGLTF_TYPE_VEC4)
                    {
                        SetErrorMessage(strfmt("Mesh attribute processing error. COLOR accessor expected to be type TINYGLTF_TYPE_VEC4. Actual type is [%s].",
                                             TinyGLTFTypeToString(accessor.componentType).c_str()));
                        return false;
                    }
                    if (accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT &&
                        accessor.componentType != TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
                    {
                        SetErrorMessage(strfmt("Mesh attribute processing error. COLOR accessor expected to be component type TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT or TINYGLTF_COMPONENT_TYPE_FLOAT. Actual type is [%s].",
                                             TinyGLTFComponentTypeToString(accessor.componentType).c_str()));
                        return false;
                    }
                    mw3dPrim.Colors.resize(accessor.count);

                    if (bufferView.byteStride == 0)
                    {
                        if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
                        {
                            memcpy(&mw3dPrim.Colors[0],
                                   &buffer.data[startingOffset],
                                   accessor.count * sizeof(DirectX::XMFLOAT4));
                        }
                        if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
                        {
                            for (int k = 0; k < accessor.count; k++)
                            {
                                uint16_t* colorPtr = reinterpret_cast<uint16_t*>(&buffer.data[startingOffset + k * 4 * sizeof(uint16_t)]);
                                mw3dPrim.Colors[k].x = (float)colorPtr[0] / 65535.0f;
                                mw3dPrim.Colors[k].y = (float)colorPtr[1] / 65535.0f;
                                mw3dPrim.Colors[k].z = (float)colorPtr[2] / 65535.0f;
                                mw3dPrim.Colors[k].w = (float)colorPtr[3] / 65535.0f;
                            }
                        }
                    }
                    else
                    {
                        if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
                        {
                            for (int k = 0; k < accessor.count; k++)
                            {
                                void* pData = &buffer.data[startingOffset + bufferView.byteStride * k];
                                memcpy(&mw3dPrim.Colors[k],
                                       pData,
                                       sizeof(DirectX::XMFLOAT4));
                            }
                        }
                        if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
                        {
                            for (int k = 0; k < accessor.count; k++)
                            {
                                uint16_t* colorPtr = reinterpret_cast<uint16_t*>(&buffer.data[startingOffset + k * bufferView.byteStride]);
                                mw3dPrim.Colors[k].x = (float)colorPtr[0] / 65535.0f;
                                mw3dPrim.Colors[k].y = (float)colorPtr[1] / 65535.0f;
                                mw3dPrim.Colors[k].z = (float)colorPtr[2] / 65535.0f;
                                mw3dPrim.Colors[k].w = (float)colorPtr[3] / 65535.0f;
                            }
                        }
                    }
                }

                if (attribute == "JOINTS_0")
                {
                    if (accessor.type != TINYGLTF_TYPE_VEC4)
                    {
                        SetErrorMessage(strfmt("Mesh attribute processing error. JOINTS accessor expected to be type TINYGLTF_TYPE_VEC4. Actual type is [%s].",
                                             TinyGLTFTypeToString(accessor.componentType).c_str()));
                        return false;
                    }

                    if (accessor.componentType != TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE &&
                        accessor.componentType != TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
                    {
                        SetErrorMessage(strfmt("Mesh attribute processing error. COLOR accessor expected to be component type TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE or TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT. Actual type is [%s].",
                                             TinyGLTFComponentTypeToString(accessor.componentType).c_str()));
                        return false;
                    }

                    mw3dPrim.JointIndices.resize(accessor.count);

                    if (bufferView.byteStride == 0)
                    {
                        if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
                        {
                            for (int k = 0; k < accessor.count; k++)
                            {
                                uint8_t* jointsPtr = reinterpret_cast<uint8_t*>(&buffer.data[startingOffset + k * 4 * sizeof(uint8_t)]);
                                for (int l = 0; l < 4; l++)
                                {
                                    uint8_t* jointPtr = jointsPtr + l;
                                    mw3dPrim.JointIndices[k].JointIndex[l] = *jointPtr;
                                }
                            }
                        }
                        if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
                        {
                            for (int k = 0; k < accessor.count; k++)
                            {
                                uint16_t* jointsPtr = reinterpret_cast<uint16_t*>(&buffer.data[startingOffset + k * 4 * sizeof(uint16_t)]);
                                for (int l = 0; l < 4; l++)
                                {
                                    uint16_t* jointPtr = jointsPtr + l;
                                    mw3dPrim.JointIndices[k].JointIndex[l] = *jointPtr;
                                }
                            }
                        }
                    }
                    else
                    {
                        if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
                        {
                            for (int k = 0; k < accessor.count; k++)
                            {
                                uint8_t* jointsPtr = reinterpret_cast<uint8_t*>(&buffer.data[startingOffset + k * bufferView.byteStride]);
                                for (int l = 0; l < 4; l++)
                                {
                                    uint8_t* jointPtr = jointsPtr + l;
                                    mw3dPrim.JointIndices[k].JointIndex[l] = *jointPtr;
                                }
                            }
                        }
                        else //UNSIGNED SHORT
                        {
                            for (int k = 0; k < accessor.count; k++)
                            {
                                uint16_t* jointsPtr = reinterpret_cast<uint16_t*>(&buffer.data[startingOffset + k * bufferView.byteStride]);
                                for (int l = 0; l < 4; l++)
                                {
                                    uint16_t* jointPtr = jointsPtr + l;
                                    mw3dPrim.JointIndices[k].JointIndex[l] = *jointPtr;
                                }
                            }
                        }
                    }
                }
                if (attribute == "WEIGHTS_0")
                {
                    if (accessor.type != TINYGLTF_TYPE_VEC4)
                    {
                        SetErrorMessage(strfmt("Mesh attribute processing error. WEIGHTS accessor expected to be type TINYGLTF_TYPE_VEC4. Actual type is [%s].",
                                             TinyGLTFTypeToString(accessor.componentType).c_str()));
                        return false;
                    }

                    if (accessor.componentType != TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE &&
                        accessor.componentType != TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT &&
                        accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT)
                    {
                        SetErrorMessage(strfmt("Mesh attribute processing error. WEIGHTS accessor expected to be component type TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE or TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT or TINYGLTF_COMPONENT_TYPE_FLOAT. Actual type is [%s].",
                                             TinyGLTFComponentTypeToString(accessor.componentType).c_str()));
                        return false;
                    }

                    mw3dPrim.JointWeights.resize(accessor.count);

                    if (bufferView.byteStride == 0)
                    {
                        if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
                        {
                            for (int k = 0; k < accessor.count; k++)
                            {
                                uint8_t* weightsPtr = reinterpret_cast<uint8_t*>(&buffer.data[startingOffset + k * 4 * sizeof(uint8_t)]);
                                for (int l = 0; l < 4; l++)
                                {
                                    uint8_t* weightPtr = weightsPtr + l;
                                    float result = *weightPtr;
                                    result /= 255.0f;
                                    mw3dPrim.JointWeights[k].JointWeight[l] = result;
                                }
                            }
                        }
                        if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
                        {
                            for (int k = 0; k < accessor.count; k++)
                            {
                                uint16_t* weightsPtr = reinterpret_cast<uint16_t*>(&buffer.data[startingOffset + k * 4 * sizeof(uint16_t)]);
                                for (int l = 0; l < 4; l++)
                                {
                                    uint16_t* weightPtr = weightsPtr + l;
                                    float result = *weightPtr;
                                    result /= 65535.0f;
                                    mw3dPrim.JointWeights[k].JointWeight[l] = result;
                                }
                            }
                        }
                        if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
                        {
                            memcpy(&mw3dPrim.JointWeights[0],
                                   &buffer.data[startingOffset],
                                   accessor.count * sizeof(JointWeightsVec4));
                        }
                    }
                    else //Interleaved
                    {
                        if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
                        {
                            for (int k = 0; k < accessor.count; k++)
                            {
                                uint8_t* weightsPtr = reinterpret_cast<uint8_t*>(&buffer.data[startingOffset + k * bufferView.byteStride]);
                                for (int l = 0; l < 4; l++)
                                {
                                    uint8_t* weightPtr = weightsPtr + l;
                                    float result = *weightPtr;
                                    result /= 255.0f;
                                    mw3dPrim.JointWeights[k].JointWeight[l] = result;
                                }
                            }
                        }
                        if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
                        {
                            for (int k = 0; k < accessor.count; k++)
                            {
                                uint16_t* weightsPtr = reinterpret_cast<uint16_t*>(&buffer.data[startingOffset + k * bufferView.byteStride]);
                                for (int l = 0; l < 4; l++)
                                {
                                    uint16_t* weightPtr = weightsPtr + l;
                                    float result = *weightPtr;
                                    result /= 65535.0f;
                                    mw3dPrim.JointWeights[k].JointWeight[l] = result;
                                }
                            }
                        }
                        if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
                        {
                            for (int k = 0; k < accessor.count; k++)
                            {
                                float* weightsPtr = reinterpret_cast<float*>(&buffer.data[startingOffset + k * bufferView.byteStride]);
                                for (int l = 0; l < 4; l++)
                                {
                                    float* weightPtr = weightsPtr + l;
                                    mw3dPrim.JointWeights[k].JointWeight[l] = *weightPtr;
                                }
                            }
                        }
                    }
                }
            }

            //get indices
            mw3dPrim.Indices.resize(mw3dPrim.Positions.size());
            if (gltfPrim.indices == -1)
            {
                for (int k = 0; k < mw3dPrim.Positions.size(); k += 3)
                {
                    mw3dPrim.Indices[k] = k;
                    mw3dPrim.Indices[k + 1] = k + 2;
                    mw3dPrim.Indices[k + 2] = k + 1;

                }
            }
            else
            {
                auto& indexAccessor = gltfModel.accessors[gltfPrim.indices];
                auto& bufferView = gltfModel.bufferViews[indexAccessor.bufferView];
                auto& buffer = gltfModel.buffers[bufferView.buffer];

                if (indexAccessor.type != TINYGLTF_TYPE_SCALAR)
                {
                    SetErrorMessage(strfmt("Mesh attribute processing error. INDEX accessor expected to be type TINYGLTF_TYPE_SCALAR. Actual type is [%s].",
                                         TinyGLTFTypeToString(indexAccessor.componentType).c_str()));
                    return false;
                }
                {
                    int startingOffset = bufferView.byteOffset + indexAccessor.byteOffset;

                    mw3dPrim.Indices.resize(indexAccessor.count);
                    if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
                    {
                        memcpy(&mw3dPrim.Indices[0],
                               &buffer.data[startingOffset],
                               bufferView.byteLength);

                        for (int j = 0; j < mw3dPrim.Indices.size(); j += 3)
                        {
                            std::swap(mw3dPrim.Indices[j],
                                      mw3dPrim.Indices[j + 2]);
                        }
                    }
                    if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
                    {
                        for (int j = 0; j < indexAccessor.count; j++)
                        {
                            uint16_t* actualIndex = reinterpret_cast<uint16_t*>(&buffer.data[startingOffset + 2 * j]);
                            mw3dPrim.Indices[j] = *actualIndex; //this is ghetto i'll figure out a better solution later
                        }
                        for (int j = 0; j < indexAccessor.count; j += 3)
                        {
                            std::swap(mw3dPrim.Indices[j],
                                      mw3dPrim.Indices[j + 2]);
                        }
                    }
                    if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
                    {
                        for (int j = 0; j < indexAccessor.count; j++)
                        {
                            uint8_t* actualIndex = reinterpret_cast<uint8_t*>(&buffer.data[startingOffset + j]);
                            mw3dPrim.Indices[j] = *actualIndex; //this is ghetto i'll figure out a better solution later
                        }
                        for (int j = 0; j < indexAccessor.count; j += 3)
                        {
                            std::swap(mw3dPrim.Indices[j],
                                      mw3dPrim.Indices[j + 2]);
                        }
                    }
                    if (indexAccessor.componentType != TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT &&
                        indexAccessor.componentType != TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT &&
                        indexAccessor.componentType != TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
                    {
                        SetErrorMessage(strfmt("Mesh attribute processing error. INDEX accessor expected to be component type TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE or TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT or TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT. Actual type is [%s].",
                                             TinyGLTFComponentTypeToString(indexAccessor.componentType).c_str()));
                        return false;
                    }
                }
            }

        }
    }

    //go and fix things like missing normals
    for (auto& mesh : m_ParseModel->Meshes)
    {
        for (auto& prim : mesh.Primitives)
        {
            if (prim.Normals.size() == 0)
            {
                if (prim.Positions.size() != prim.Indices.size())
                {
                    ParsedGLTFModel::PrimitiveData oldPrim = prim;
                    prim.Positions.resize(prim.Indices.size());
                    if (prim.Colors.size() > 0)
                    {
                        prim.Colors.resize(prim.Indices.size());
                    }
                    if (prim.TextureCoordinates.size() > 0)
                    {
                        prim.TextureCoordinates.resize(prim.Indices.size());
                    }
                    if (prim.JointIndices.size() > 0)
                    {
                        prim.JointIndices.resize(prim.Indices.size());
                    }
                    if (prim.JointWeights.size() > 0)
                    {
                        prim.JointWeights.resize(prim.Indices.size());
                    }

                    for (int i = 0; i < prim.Indices.size(); i += 1)
                    {
                        int i1 = prim.Indices[i];
                        prim.Positions[i] = oldPrim.Positions[i1];
                        if (prim.Colors.size() > 0)
                        {
                            prim.Colors[i] = oldPrim.Colors[i1];
                        }
                        if (prim.TextureCoordinates.size() > 0)
                        {
                            prim.TextureCoordinates[i] = oldPrim.TextureCoordinates[i1];
                        }
                        if (prim.JointIndices.size() > 0)
                        {
                            prim.JointIndices[i] = oldPrim.JointIndices[i1];
                        }
                        if (prim.JointWeights.size() > 0)
                        {
                            prim.JointWeights[i] = oldPrim.JointWeights[i1];
                        }
                    }
                    for (int i = 0; i < prim.Indices.size(); i++)
                    {
                        prim.Indices[i] = i;
                    }
                }

                prim.Normals.resize(prim.Positions.size());
                for (int i = 0; i < prim.Indices.size(); i += 3)
                {
                    int i1 = prim.Indices[i];
                    int i2 = prim.Indices[i + 1];
                    int i3 = prim.Indices[i + 2];

                    DirectX::XMFLOAT3 p1 = prim.Positions[i1];
                    DirectX::XMFLOAT3 p2 = prim.Positions[i2];
                    DirectX::XMFLOAT3 p3 = prim.Positions[i3];

                    DirectX::XMVECTOR p1_vec = DirectX::XMLoadFloat3(&p1);
                    DirectX::XMVECTOR p2_vec = DirectX::XMLoadFloat3(&p2);
                    DirectX::XMVECTOR p3_vec = DirectX::XMLoadFloat3(&p3);


                    DirectX::XMVECTOR v1 = DirectX::XMVectorSubtract(p3_vec, p1_vec);
                    DirectX::XMVECTOR v2 = DirectX::XMVectorSubtract(p2_vec, p1_vec);
                    DirectX::XMVECTOR normalVec = DirectX::XMVector3Cross(v1, v2);
                    normalVec = DirectX::XMVector3Normalize(normalVec);
                    DirectX::XMFLOAT3 normal;
                    DirectX::XMStoreFloat3(&normal, normalVec);
                    prim.Normals[i1] = normal;
                    prim.Normals[i2] = normal;
                    prim.Normals[i3] = normal;
                }
            }
        }
    }
    return true;
}

bool EZGLTF::GLTFLoader::ProcessAnimations()
{
    auto& gltfModel = m_GLTFModel;

    for (auto& gltfAnim : gltfModel.animations)
    {
        AnimationClip animationClip;
        animationClip.Name = gltfAnim.name;

        for (auto& gltfChannel : gltfAnim.channels)
        {
            AnimationChannel* ptrMW3DAnimationChannel = nullptr;
            //Doing this because there will be different channels for scaling/rotation/translation and I want to combine them all into the same animation channel
            for (int i = 0; i < animationClip.Channels.size(); i++)
            {
                if (animationClip.Channels[i].m_TargetNode == m_LookupOldNodeToNewNodeIndex[gltfChannel.target_node])
                {
                    ptrMW3DAnimationChannel = &animationClip.Channels[i];
                }
            }
            if (ptrMW3DAnimationChannel == nullptr)
            {
                animationClip.Channels.push_back(AnimationChannel());
                ptrMW3DAnimationChannel = &animationClip.Channels[animationClip.Channels.size() - 1];
                ptrMW3DAnimationChannel->m_TargetNode = m_LookupOldNodeToNewNodeIndex[gltfChannel.target_node];
            }
            auto& gltfSampler = gltfAnim.samplers[gltfChannel.sampler];

            auto& inputAccessor = gltfModel.accessors[gltfSampler.input];
            auto& inputBufferView = gltfModel.bufferViews[inputAccessor.bufferView];
            auto& inputBuffer = gltfModel.buffers[inputBufferView.buffer];
            auto& outputAccessor = gltfModel.accessors[gltfSampler.output];
            auto& outputBufferView = gltfModel.bufferViews[outputAccessor.bufferView];
            auto& outputBuffer = gltfModel.buffers[outputBufferView.buffer];

            if (inputAccessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT)
            {
                SetErrorMessage(strfmt("Animation channel input accessor component type invalid. Expected type TINYGLTF_COMPONENT_TYPE_FLOAT. Actual type is [%s].",
                                     TinyGLTFComponentTypeToString(inputAccessor.componentType).c_str()));
                return false;
            }
            if (inputAccessor.type != TINYGLTF_TYPE_SCALAR)
            {
                SetErrorMessage(strfmt("Animation channel input accessor type invalid. Expected type TINYGLTF_TYPE_SCALAR. Actual type is [%s].",
                                     TinyGLTFTypeToString(inputAccessor.componentType).c_str()));
                return false;
            }
            if (outputAccessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT)
            {
                SetErrorMessage(strfmt("Animation channel output accessor component type invalid. Expected type TINYGLTF_COMPONENT_TYPE_FLOAT. Actual type is [%s].",
                                     TinyGLTFComponentTypeToString(outputAccessor.componentType).c_str()));
                return false;
            }

            std::string interpolationTypeString = gltfSampler.interpolation;
            AnimationInterpolationType interpolationType = AnimationInterpolationType::UNDEFINED;
            if (interpolationTypeString == "LINEAR")
            {
                interpolationType = AnimationInterpolationType::LINEAR;
            }
            if (interpolationTypeString == "CUBICSPLINE")
            {
                interpolationType = AnimationInterpolationType::CUBLICSPLINE;
            }
            if (interpolationTypeString == "STEP")
            {
                interpolationType = AnimationInterpolationType::STEP;
            }

            if (inputAccessor.count != outputAccessor.count)
            {
                SetErrorMessage(strfmt("Animation channel has an input accessor count mismatch with output accessor count. This occurred for animation channel with target path [%s].",
                                     gltfChannel.target_path.c_str()));
                return false;
            }

            if (inputBufferView.byteStride != 0 || outputBufferView.byteStride != 0)
            {
                SetErrorMessage("One or more animation channels have a byte stride for either the input or output buffer view. This is not yet implemented.");
                return false;
            }

            if (gltfChannel.target_path == "rotation")
            {
                if (outputAccessor.type != TINYGLTF_TYPE_VEC4)
                {
                    SetErrorMessage(strfmt("Animation channel for rotation expects output accessor type of TINYGLTF_TYPE_VEC4. Actual type is [%s].",
                                         TinyGLTFTypeToString(outputAccessor.type).c_str()));
                    return false;
                }

                ptrMW3DAnimationChannel->m_RotationInterpolationType = interpolationType;
                ptrMW3DAnimationChannel->m_RotationKeys.resize(inputAccessor.count);

                for (int i = 0; i < inputAccessor.count; i++)
                {
                    void* inputOffset = &inputBuffer.data[inputBufferView.byteOffset + inputAccessor.byteOffset + i * sizeof(float)];
                    ptrMW3DAnimationChannel->m_RotationKeys[i].m_AnimationTime = *(float*)inputOffset;
                }

                for (int i = 0; i < outputAccessor.count; i++)
                {
                    void* outputOffset = &outputBuffer.data[outputBufferView.byteOffset + outputAccessor.byteOffset + i * sizeof(DirectX::XMFLOAT4)];
                    ptrMW3DAnimationChannel->m_RotationKeys[i].m_KeyData = *(DirectX::XMFLOAT4*)outputOffset;
                }
            }

            if (gltfChannel.target_path == "scale")
            {
                if (outputAccessor.type != TINYGLTF_TYPE_VEC3)
                {
                    SetErrorMessage(strfmt("Animation channel for scale expects output accessor type of TINYGLTF_TYPE_VEC3. Actual type is [%s].",
                                         TinyGLTFTypeToString(outputAccessor.type).c_str()));
                    return false;
                }

                ptrMW3DAnimationChannel->m_ScaleInterpolationType = interpolationType;
                ptrMW3DAnimationChannel->m_ScaleKeys.resize(inputAccessor.count);

                for (int i = 0; i < inputAccessor.count; i++)
                {
                    void* inputOffset = &inputBuffer.data[inputBufferView.byteOffset + inputAccessor.byteOffset + i * sizeof(float)];
                    ptrMW3DAnimationChannel->m_ScaleKeys[i].m_AnimationTime = *(float*)inputOffset;
                }

                for (int i = 0; i < outputAccessor.count; i++)
                {
                    void* outputOffset = &outputBuffer.data[outputBufferView.byteOffset + outputAccessor.byteOffset + i * sizeof(DirectX::XMFLOAT3)];
                    ptrMW3DAnimationChannel->m_ScaleKeys[i].m_KeyData = *(DirectX::XMFLOAT3*)outputOffset;
                }
            }
            if (gltfChannel.target_path == "translation")
            {
                if (outputAccessor.type != TINYGLTF_TYPE_VEC3)
                {
                    SetErrorMessage(strfmt("Animation channel for translation expects output accessor type of TINYGLTF_TYPE_VEC3. Actual type is [%s].",
                                         TinyGLTFTypeToString(outputAccessor.type).c_str()));
                    return false;
                }

                ptrMW3DAnimationChannel->m_TranslationInterpolationType = interpolationType;
                ptrMW3DAnimationChannel->m_TranslationKeys.resize(inputAccessor.count);

                for (int i = 0; i < inputAccessor.count; i++)
                {
                    void* inputOffset = &inputBuffer.data[inputBufferView.byteOffset + inputAccessor.byteOffset + i * sizeof(float)];
                    ptrMW3DAnimationChannel->m_TranslationKeys[i].m_AnimationTime = *(float*)inputOffset;
                }

                for (int i = 0; i < outputAccessor.count; i++)
                {
                    void* outputOffset = &outputBuffer.data[outputBufferView.byteOffset + outputAccessor.byteOffset + i * sizeof(DirectX::XMFLOAT3)];
                    ptrMW3DAnimationChannel->m_TranslationKeys[i].m_KeyData = *(DirectX::XMFLOAT3*)outputOffset;
                }
            }
            if (gltfChannel.target_path == "weights")
            {
                //TODO: Need to test this
                if (outputAccessor.type != TINYGLTF_TYPE_SCALAR)
                {
                    SetErrorMessage(strfmt("Animation channel for weights expects output accessor type of TINYGLTF_TYPE_SCALAR. Actual type is [%s].",
                                         TinyGLTFTypeToString(outputAccessor.type).c_str()));
                    return false;
                }

                ptrMW3DAnimationChannel->m_WeightInterpolationType = interpolationType;
                ptrMW3DAnimationChannel->m_WeightKeys.resize(inputAccessor.count);

                for (int i = 0; i < inputAccessor.count; i++)
                {
                    void* inputOffset = &inputBuffer.data[inputBufferView.byteOffset + inputAccessor.byteOffset + i * sizeof(float)];
                    ptrMW3DAnimationChannel->m_WeightKeys[i].m_AnimationTime = *(float*)inputOffset;
                }

                for (int i = 0; i < outputAccessor.count; i++)
                {
                    void* outputOffset = &outputBuffer.data[outputBufferView.byteOffset + outputAccessor.byteOffset + i * sizeof(float)];
                    ptrMW3DAnimationChannel->m_WeightKeys[i].m_KeyData = *(float*)outputOffset;
                }
            }
        }

        for (auto& channel : animationClip.Channels)
        {
            for (auto& key : channel.m_TranslationKeys)
            {
                animationClip.Duration = std::max(animationClip.Duration, key.m_AnimationTime);
            }
            for (auto& key : channel.m_RotationKeys)
            {
                animationClip.Duration = std::max(animationClip.Duration, key.m_AnimationTime);
            }
            for (auto& key : channel.m_ScaleKeys)
            {
                animationClip.Duration = std::max(animationClip.Duration, key.m_AnimationTime);
            }
            for (auto& key : channel.m_WeightKeys)
            {
                animationClip.Duration = std::max(animationClip.Duration, key.m_AnimationTime);
            }
        }
        m_ParseModel->Animations.push_back(animationClip);
    }

    return true;
}

void EZGLTF::GLTFLoader::BuildGlobalSpacePositionsAndIndices()
{
    //TODO: Decide what to do with rigged meshes - just skip this completely maybe? The bones jack stuff up
    std::function<void(ParsedGLTFModel& inModel,
                       SkeletonNode& inNode,
                       DirectX::SimpleMath::Matrix inTransform)> ProcessGlobalSpaceVerticesFromNodeAndChildren;

    ProcessGlobalSpaceVerticesFromNodeAndChildren = [&](ParsedGLTFModel& inModel,
                                                        SkeletonNode& inNode,
                                                        DirectX::SimpleMath::Matrix inTransform)
        {
            auto& outVertices = inModel.PositionsInGlobalSpace;
            auto& outTriangles = inModel.IndicesForGlobalSpacePositions;

            DirectX::SimpleMath::Matrix adjustedTransform = inNode.DefaultLocalTransform * inTransform;
            for (int meshIndex : inNode.MeshIndices)
            {
                auto& mesh = inModel.Meshes[meshIndex];
                for (auto& prim : mesh.Primitives)
                {
                    int vertexOffset = outVertices.size();
                    for (int i = 0; i < prim.Positions.size(); i++)
                    {
                        auto& pos = prim.Positions[i];

                        DirectX::XMVECTOR vIn = DirectX::XMLoadFloat3(&pos);
                        DirectX::XMVECTOR vOut = DirectX::XMVector3TransformCoord(vIn, adjustedTransform);
                        DirectX::XMFLOAT3 fixedPos;
                        DirectX::XMStoreFloat3(&fixedPos, vOut);

                        outVertices.push_back(fixedPos);
                    }

                    for (int i = 0; i < prim.Indices.size(); i += 3)
                    {
                        auto index1 = prim.Indices[i] + vertexOffset;
                        auto index2 = prim.Indices[i + 2] + vertexOffset;
                        auto index3 = prim.Indices[i + 1] + vertexOffset; //For some reason I have to send the triangles backwards for Jolt?
                        outTriangles.push_back(DirectX::XMINT3(index1, index2, index3));
                    }
                }
            }

            for (int j = 0; j < inNode.ChildrenIndices.size(); j++)
            {
                const int nodeIndex = inNode.ChildrenIndices[j];
                ProcessGlobalSpaceVerticesFromNodeAndChildren(inModel,
                                                              inModel.Nodes[nodeIndex],
                                                              adjustedTransform);
            }
        };

    for (int i = 0; i < m_ParseModel->Nodes.size(); i++)
    {
        auto& node = m_ParseModel->Nodes[i];
        if (node.ParentIndex == -1) //Root node
        {
            ProcessGlobalSpaceVerticesFromNodeAndChildren(*m_ParseModel, 
                                                          m_ParseModel->Nodes[i], 
                                                          Matrix::Identity);
        }
    }
}

void EZGLTF::GLTFLoader::SetErrorMessage(string msg)
{
    m_ErrorMessage = msg + "\n" + "File: " + m_FilePath;
}
