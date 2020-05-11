#ifndef MODEL_H
#define MODEL_H

#include <fbxsdk.h>

#include "D3D.h"
#include "Texture.h"
#include "ResourceManager.h"

#include <string>

#define ASSERT_RESULT() if (FAILED(result)) return false;
#define RELEASE(P_OBJ) if(P_OBJ) { P_OBJ->Release(); P_OBJ = nullptr; } 
#define SHUTDOWN(RES) if(RES) { RES->Shutdown(); delete RES; RES = nullptr; }

typedef struct textured_vertex
{
	XMFLOAT3 position;
	XMFLOAT2 texture;
	XMFLOAT3 normal;
}Vertex;

// YES IM FULLY AWARE OF HOW HORRIBLE THIS IS, leave me alone, this is for later
class Model
	: public IShutdown
{
public:
	Model();
	Model(const Model&) = delete;
	Model& operator=(const Model&) = delete;

	bool Initialize(std::string modelDescriptorPath);
	void Shutdown();

	constexpr int GetIndexCount() noexcept { return m_IndexCount; }
	constexpr ID3D11Buffer* GetVertexBuffer() noexcept { return m_pVertexBuffer; }
	constexpr ID3D11Buffer* GetIndexBuffer() noexcept { return m_pIndexBuffer; }

private:
	bool InitializeBuffers(ID3D11Device* pDevice, std::string modelFilepath, std::string modelName, std::string uvChannel);

	ID3D11Buffer* m_pVertexBuffer;
	ID3D11Buffer* m_pIndexBuffer;
	int m_VertexCount;
	int m_IndexCount;
	bool m_LoadStatus;

public:
	static void LoadFBX(LPCSTR modelPath, LPCSTR modelName, int& vertexCount, int& indexCount, Vertex*& pVertices, ULONG*& pIndices, int& errorCounter, LPCTSTR uvchannel = "UVChannel_1")
	{
		FbxManager* pSdk = FbxManager::Create();
		FbxIOSettings* pSettings = FbxIOSettings::Create(pSdk, IOSROOT);
		FbxImporter* pImporter = FbxImporter::Create(pSdk, "");

		errorCounter = 0;
		bool modelFound = false;

		if (!pImporter->Initialize(modelPath, -1, pSdk->GetIOSettings()))
		{
			LOGGER->Log<LOG_ERROR>("Call for FbxImporter::Initialize() failed: \n \t");
			errorCounter++;
		}

		FbxScene* pScene = FbxScene::Create(pSdk, "LoadingScene");
		pImporter->Import(pScene);

		auto pGeometryConverter = new FbxGeometryConverter(pSdk);
		pGeometryConverter->Triangulate(pScene, false);

		delete pGeometryConverter;

		FbxNode* pRootNode = pScene->GetRootNode();
		if (pRootNode)
		{
			for (int k = 0; k < pRootNode->GetChildCount(); ++k)
			{
				FbxNode* pTempNode = pRootNode->GetChild(k);

				if (strcmp(pTempNode->GetName(), modelName) == 0)
				{
					LOGGER->Log<LOG_INFO>("Model found!\n");

					modelFound = true;
					FbxMesh* pModelMesh = pTempNode->GetMesh();

					vertexCount = pModelMesh->GetPolygonVertexCount();
					indexCount = vertexCount;

					pVertices = new Vertex[vertexCount];
					pIndices = new ULONG[indexCount];

					ULONG vC = 0;
					for (int i = 0; i < pModelMesh->GetPolygonCount(); ++i)
					{
						for (int j = 2; j >= 0; --j)
						{
							// Get vertex position
							int vertex = pModelMesh->GetPolygonVertex(i, j);
							FbxVector4 pos = pModelMesh->GetControlPointAt(vertex);

							Vertex v;
							v.position = { float(pos.mData[0]), float(pos.mData[1]), -float(pos.mData[2]) };

							// Get Vertex normal
							FbxVector4 normal;
							pModelMesh->GetPolygonVertexNormal(i, j, normal);
							normal.Normalize();

							v.normal = { float(normal.mData[0]), float(normal.mData[1]), -float(normal.mData[2]) };

							// Get texture coordinates
							FbxVector2 uv;
							bool unmapped;

							pModelMesh->GetPolygonVertexUV(i, j, (char*)uvchannel, uv, unmapped);

							v.texture.x = float(uv.mData[0]);
							v.texture.y = float(1.f - uv.mData[1]);

							pVertices[vC] = v;
							pIndices[vC] = vC;

							vC++;
						}
					}
					pModelMesh->Destroy();
				}
			}
		}

		if (!modelFound)
			errorCounter++;

		pScene->Destroy(true);
		pImporter->Destroy();
		pSettings->Destroy();
		pSdk->Destroy();
	}
};


#endif // !MODEL_H

