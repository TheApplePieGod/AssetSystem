#include "pch.h"
#include "../assetLoader.h"
#include "assetTypes.h"

#include "pch.h"
#include "../assetLoader.h"
#include "assetTypes.h"

// multithreading not supported
//void GetPolygonData(u32 StartingPolygon, u32 EndingPolygon, u32 DirectArrayIndex, std::vector<assetTypes::vertex>* LocalVertexArray, FbxMesh* pMesh)
//{
//	u32 ArrayIndex = 0;
//	FbxVector4* lControlPoints = pMesh->GetControlPoints();
//	for (u32 j = StartingPolygon; j < EndingPolygon; j++)
//	{
//		u32 NumVertices = pMesh->GetPolygonSize(j);
//		//if (NumVertices != 3) // must be triangulated
//		Assert(NumVertices == 3);
//
//		for (u32 k = 0; k < NumVertices; k++)
//		{
//			assetTypes::vertex vert = assetTypes::vertex();
//			LocalVertexArray->push_back(vert);
//
//			//vertices data
//			u32 ControlPointIndex = pMesh->GetPolygonVertex(j, k);
//			FbxVector4 Vertex = lControlPoints[ControlPointIndex];
//
//			LocalVertexArray->at(ArrayIndex).position = assetTypes::v3{ (float)Vertex[0], (float)Vertex[1], (float)Vertex[2] };
//
//			//UV data
//			u32 ElementUVCount = pMesh->GetElementUVCount();
//			for (u32 l = 0; l < ElementUVCount; ++l)
//			{
//				FbxGeometryElementUV* leUV = pMesh->GetElementUV(l);
//				FbxVector2 UV;
//				switch (leUV->GetMappingMode())
//				{
//				default:
//				{ }	break;
//
//				case FbxGeometryElement::eByControlPoint:
//				{
//					switch (leUV->GetReferenceMode())
//					{
//					case FbxGeometryElement::eDirect:
//					{
//						UV = leUV->GetDirectArray().GetAt(ControlPointIndex);
//					} break;
//
//					case FbxGeometryElement::eIndexToDirect:
//					{
//						int id = leUV->GetIndexArray().GetAt(ControlPointIndex);
//						UV = leUV->GetDirectArray().GetAt(id);
//					} break;
//
//					default:
//					{ }	break; // other reference modes not shown here!
//					}
//				} break;
//
//				case FbxGeometryElement::eByPolygonVertex:
//				{
//					int lTextureUVIndex = pMesh->GetTextureUVIndex(j, k);
//					switch (leUV->GetReferenceMode())
//					{
//					case FbxGeometryElement::eDirect:
//					case FbxGeometryElement::eIndexToDirect:
//					{
//						UV = leUV->GetDirectArray().GetAt(lTextureUVIndex);
//					} break;
//
//					default:
//					{ }	break; // other reference modes not shown here!
//					}
//				} break;
//
//				case FbxGeometryElement::eByPolygon: // doesn't make much sense for UVs
//				case FbxGeometryElement::eAllSame:   // doesn't make much sense for UVs
//				case FbxGeometryElement::eNone:       // doesn't make much sense for UVs
//				{ }	break;
//				}
//
//				LocalVertexArray->at(ArrayIndex).uv = assetTypes::v2{ (float)UV[0], (float)UV[1] };
//			}
//
//			// normal data
//			u32 ElementNormalCount = pMesh->GetElementNormalCount();
//			if (ElementNormalCount == 0)
//				pMesh->GenerateNormals();
//			for (u32 l = 0; l < ElementNormalCount; ++l)
//			{
//				FbxVector4 Normal;
//				FbxGeometryElementNormal* lNormalElement = pMesh->GetElementNormal(l);
//				if (lNormalElement)
//				{
//					if (lNormalElement->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
//					{
//						switch (lNormalElement->GetReferenceMode())
//						{
//						case FbxGeometryElement::eDirect:
//						{
//							Normal = lNormalElement->GetDirectArray().GetAt(DirectArrayIndex + ArrayIndex);
//						} break;
//
//						case FbxGeometryElement::eIndexToDirect:
//						{
//							int id = lNormalElement->GetIndexArray().GetAt(DirectArrayIndex + ArrayIndex);
//							Normal = lNormalElement->GetDirectArray().GetAt(id);
//						} break;
//
//						default:
//						{ }	break;
//						}
//					}
//				}
//
//				// storing normals as float3s
//				LocalVertexArray->at(ArrayIndex).normal = assetTypes::v3{ (float)Normal[0], (float)Normal[1], (float)Normal[2] };
//			}
//
//			// tangent data
//			//u32 ElementTangentCount = pMesh->GetElementTangentCount();
//			//if (ElementTangentCount == 0)
//			//	pMesh->GenerateTangentsDataForAllUVSets();
//			//for (u32 l = 0; l < ElementTangentCount; ++l)
//			//{
//			//	FbxVector4 Tangent;
//			//	FbxGeometryElementTangent* lTangentElement = pMesh->GetElementTangent(l);
//			//	if (lTangentElement)
//			//	{
//			//		if (lTangentElement->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
//			//		{
//			//			switch (lTangentElement->GetReferenceMode())
//			//			{
//			//			case FbxGeometryElement::eDirect:
//			//			{
//			//				Tangent = lTangentElement->GetDirectArray().GetAt(DirectArrayIndex + ArrayIndex); //?
//			//			} break;
//
//			//			case FbxGeometryElement::eIndexToDirect:
//			//			{
//			//				int id = lTangentElement->GetIndexArray().GetAt(DirectArrayIndex + ArrayIndex);
//			//				Tangent = lTangentElement->GetDirectArray().GetAt(id);
//			//			} break;
//
//			//			default:
//			//			{ }	break;
//			//			}
//			//		}
//			//	}
//
//			//	// storing tangent as float3s
//			//	VertexArray[ArrayIndex].tx = (float)Tangent[0];
//			//	VertexArray[ArrayIndex].ty = (float)Tangent[1];
//			//	VertexArray[ArrayIndex].tz = (float)Tangent[2];
//			//}
//
//			ArrayIndex++;
//		}
//	}
//}
//
//bool assetTypes::Mesh_GetDataForWriting(char*& Out_ExtraData, char*& Out_RawData, u32& Out_ExtraDataSize, u32& Out_RawDataSize, char* FilePath)
//{
//	// Initialize the SDK manager. This object handles memory management.
//	FbxManager* lSdkManager = FbxManager::Create();
//
//	// Create the IO settings object.
//	FbxIOSettings* ios = FbxIOSettings::Create(lSdkManager, IOSROOT);
//	lSdkManager->SetIOSettings(ios);
//
//	// Create an importer using the SDK manager.
//	FbxImporter* lImporter = FbxImporter::Create(lSdkManager, "");
//
//	// Use the first argument as the filename for the importer.
//	if (!lImporter->Initialize(FilePath, -1, lSdkManager->GetIOSettings())) {
//		printf("FBX initialized failed with file %s\n", FilePath);
//		printf("Error returned: %s\n\n", lImporter->GetStatus().GetErrorString());
//		return false;
//	}
//
//	// Create a new scene so that it can be populated by the imported file.
//	FbxScene* lScene = FbxScene::Create(lSdkManager, "myScene");
//
//	// Import the contents of the file into the scene.
//	if (!lImporter->Import(lScene))
//		return false;
//
//	FbxGeometryConverter FbxConverter(lSdkManager);
//	FbxAxisSystem AxisSystem = lScene->GetGlobalSettings().GetAxisSystem();
//#ifdef ASSET_DIRECTX11
//	FbxAxisSystem::DirectX.ConvertScene(lScene);
//#endif
//	FbxConverter.Triangulate(lScene, true);
//
//	// The file is imported, so get rid of the importer.
//	lImporter->Destroy();
//
//	FbxNode* pFbxRootNode = lScene->GetRootNode();
//	if (pFbxRootNode)
//	{
//		std::vector<assetTypes::vertex> VertexArray;
//
//		int NumThreads = std::thread::hardware_concurrency();
//		if (NumThreads == 0)
//			NumThreads = 1;
//
//		for (int i = 0; i < pFbxRootNode->GetChildCount(); i++)
//		{
//			FbxNode* pFbxChildNode = pFbxRootNode->GetChild(i);
//
//			if (pFbxChildNode->GetNodeAttribute() == NULL)
//				continue;
//
//			FbxNodeAttribute::EType AttributeType = pFbxChildNode->GetNodeAttribute()->GetAttributeType();
//
//			if (AttributeType != FbxNodeAttribute::eMesh) // only compatible with mesh types
//				continue;
//
//			fbxsdk::FbxMesh* pMesh = (fbxsdk::FbxMesh*)pFbxChildNode->GetNodeAttribute();
//
//			u32 CurrentPolygon = 0;
//			u32 DirectArrayIndex = 0;
//			u32 TotalPolygons = pMesh->GetPolygonCount();
//			u32 PolygonsPerThread = (u32)(TotalPolygons / NumThreads);
//			u32 Extra = TotalPolygons - (PolygonsPerThread * NumThreads);
//
//			std::vector<std::thread> threads(NumThreads);
//			std::vector<std::vector<assetTypes::vertex>> LocalVertexArrays;
//			for (int i = 0; i < NumThreads; i++)
//			{
//				std::vector<assetTypes::vertex> LocalVertArray;
//				LocalVertexArrays.push_back(LocalVertArray);
//				threads[i] = std::thread(GetPolygonData, CurrentPolygon, CurrentPolygon + PolygonsPerThread + (i == NumThreads - 1 ? Extra : 0), DirectArrayIndex, &LocalVertexArrays[i], pMesh);
//				DirectArrayIndex += PolygonsPerThread * 3; // triangles
//				CurrentPolygon += PolygonsPerThread;
//			}
//
//			for (auto& th : threads)
//				th.join();
//
//			for (std::vector<assetTypes::vertex>& VertArray : LocalVertexArrays)
//			{
//				for (assetTypes::vertex Vert : VertArray)
//				{
//					VertexArray.push_back(Vert);
//				}
//			}
//		}
//
//		for (u32 i = 0; i < (u32)VertexArray.size(); i += 3) // calculate tangents
//		{
//			assetTypes::vertex& v0 = VertexArray[i];
//			assetTypes::vertex& v1 = VertexArray[i + 1];
//			assetTypes::vertex& v2 = VertexArray[i + 2];
//
//			assetTypes::v3 pos0 = v0.position;
//			assetTypes::v3 pos1 = v1.position;
//			assetTypes::v3 pos2 = v2.position;
//
//			assetTypes::v2 uv0 = v0.uv;
//			assetTypes::v2 uv1 = v1.uv;
//			assetTypes::v2 uv2 = v2.uv;
//
//			// Position delta
//			assetTypes::v3 deltaPos1 = pos1 - pos0;
//			assetTypes::v3 deltaPos2 = pos2 - pos0;
//
//			// UV delta
//			assetTypes::v2 deltaUV1 = uv1 - uv0;
//			assetTypes::v2 deltaUV2 = uv2 - uv0;
//
//			float r = 1.0f / (deltaUV1.u * deltaUV2.v - deltaUV1.v * deltaUV2.u);
//			assetTypes::v3 tangent = (deltaPos1 * deltaUV2.v - deltaPos2 * deltaUV1.v) * r;
//			assetTypes::v3 bitangent = (deltaPos2 * deltaUV1.u - deltaPos1 * deltaUV2.u) * r;
//
//			v0.tangent = tangent;
//			v1.tangent = tangent;
//			v2.tangent = tangent;
//
//			v0.bitangent = bitangent;
//			v1.bitangent = bitangent;
//			v2.bitangent = bitangent;
//		}
//
//		mesh_data MeshData;
//		MeshData.NumVertices = (u32)VertexArray.size();
//
//		u32 DataLength = MeshData.NumVertices * sizeof(assetTypes::vertex);
//		u32 ExtraSize = sizeof(MeshData);
//
//		// Set fields
//		Out_ExtraData = new char[ExtraSize];
//		memcpy(Out_ExtraData, &MeshData, ExtraSize);
//		Out_RawData = new char[DataLength]; // Allocate new array since std::vector auto deletes data
//		memcpy(Out_RawData, (char*)VertexArray.data(), DataLength);
//		Out_ExtraDataSize = ExtraSize;
//		Out_RawDataSize = DataLength;
//		
//		return true;
//	}
//	else
//		return false;
//}

bool assetTypes::Mesh_GetDataForWriting(char*& Out_ExtraData, char*& Out_RawData, u32& Out_ExtraDataSize, u32& Out_RawDataSize, char* FilePath)
{
	// Initialize the SDK manager. This object handles memory management.
	FbxManager* lSdkManager = FbxManager::Create();

	// Create the IO settings object.
	FbxIOSettings* ios = FbxIOSettings::Create(lSdkManager, IOSROOT);
	lSdkManager->SetIOSettings(ios);

	// Create an importer using the SDK manager.
	FbxImporter* lImporter = FbxImporter::Create(lSdkManager, "");

	// Use the first argument as the filename for the importer.
	if (!lImporter->Initialize(FilePath, -1, lSdkManager->GetIOSettings())) {
		printf("FBX initialized failed with file %s\n", FilePath);
		printf("Error returned: %s\n\n", lImporter->GetStatus().GetErrorString());
		return false;
	}

	// Create a new scene so that it can be populated by the imported file.
	FbxScene* lScene = FbxScene::Create(lSdkManager, "myScene");

	// Import the contents of the file into the scene.
	if (!lImporter->Import(lScene))
		return false;

	FbxGeometryConverter FbxConverter(lSdkManager);
	FbxAxisSystem AxisSystem = lScene->GetGlobalSettings().GetAxisSystem();
#ifdef ASSET_DIRECTX11
	FbxAxisSystem::DirectX.ConvertScene(lScene);
#endif
	FbxConverter.Triangulate(lScene, true);

	// The file is imported, so get rid of the importer.
	lImporter->Destroy();

	FbxNode* pFbxRootNode = lScene->GetRootNode();
	if (pFbxRootNode)
	{
		std::vector<assetTypes::vertex> VertexArray;
		u32 ArrayIndex = 0;
		for (int i = 0; i < pFbxRootNode->GetChildCount(); i++)
		{
			FbxNode* pFbxChildNode = pFbxRootNode->GetChild(i);

			if (pFbxChildNode->GetNodeAttribute() == NULL)
				continue;

			FbxNodeAttribute::EType AttributeType = pFbxChildNode->GetNodeAttribute()->GetAttributeType();

			if (AttributeType != FbxNodeAttribute::eMesh) // only compatible with mesh types
				continue;

			fbxsdk::FbxMesh* pMesh = (fbxsdk::FbxMesh*)pFbxChildNode->GetNodeAttribute();

			FbxVector4* lControlPoints = pMesh->GetControlPoints();

			u32 countd = pMesh->GetPolygonCount();
			for (s32 j = 0; j < pMesh->GetPolygonCount(); j++)
			{
				u32 NumVertices = pMesh->GetPolygonSize(j);
				if (NumVertices != 3) // must be triangulated
					return false;

				for (u32 k = 0; k < NumVertices; k++)
					//for (s32 k = NumVertices - 1; k >= 0; k--)
				{
					assetTypes::vertex vert = assetTypes::vertex();
					VertexArray.push_back(vert);

					//vertices data
					u32 ControlPointIndex = pMesh->GetPolygonVertex(j, k);
					FbxVector4 Vertex = lControlPoints[ControlPointIndex];

					VertexArray[ArrayIndex].position = v3{ (float)Vertex[0], (float)Vertex[1], (float)Vertex[2] };

					//UV data
					u32 ElementUVCount = pMesh->GetElementUVCount();
					for (u32 l = 0; l < ElementUVCount; ++l)
					{
						FbxGeometryElementUV* leUV = pMesh->GetElementUV(l);
						FbxVector2 UV;
						switch (leUV->GetMappingMode())
						{
						default:
						{ }	break;

						case FbxGeometryElement::eByControlPoint:
						{
							switch (leUV->GetReferenceMode())
							{
							case FbxGeometryElement::eDirect:
							{
								UV = leUV->GetDirectArray().GetAt(ControlPointIndex);
							} break;

							case FbxGeometryElement::eIndexToDirect:
							{
								int id = leUV->GetIndexArray().GetAt(ControlPointIndex);
								UV = leUV->GetDirectArray().GetAt(id);
							} break;

							default:
							{ }	break; // other reference modes not shown here!
							}
						} break;

						case FbxGeometryElement::eByPolygonVertex:
						{
							int lTextureUVIndex = pMesh->GetTextureUVIndex(j, k);
							switch (leUV->GetReferenceMode())
							{
							case FbxGeometryElement::eDirect:
							case FbxGeometryElement::eIndexToDirect:
							{
								UV = leUV->GetDirectArray().GetAt(lTextureUVIndex);
							} break;

							default:
							{ }	break; // other reference modes not shown here!
							}
						} break;

						case FbxGeometryElement::eByPolygon: // doesn't make much sense for UVs
						case FbxGeometryElement::eAllSame:   // doesn't make much sense for UVs
						case FbxGeometryElement::eNone:       // doesn't make much sense for UVs
						{ }	break;
						}

						VertexArray[ArrayIndex].uv = v2{ (float)UV[0], (float)UV[1] };
					}

					// normal data
					u32 ElementNormalCount = pMesh->GetElementNormalCount();
					if (ElementNormalCount == 0)
						pMesh->GenerateNormals();
					for (u32 l = 0; l < ElementNormalCount; ++l)
					{
						FbxVector4 Normal;
						FbxGeometryElementNormal* lNormalElement = pMesh->GetElementNormal(l);
						if (lNormalElement)
						{
							if (lNormalElement->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
							{
								switch (lNormalElement->GetReferenceMode())
								{
								case FbxGeometryElement::eDirect:
								{
									Normal = lNormalElement->GetDirectArray().GetAt(ArrayIndex);
								} break;

								case FbxGeometryElement::eIndexToDirect:
								{
									int id = lNormalElement->GetIndexArray().GetAt(ArrayIndex);
									Normal = lNormalElement->GetDirectArray().GetAt(id);
								} break;

								default:
								{ }	break;
								}
							}
						}

						// storing normals as float3s
						VertexArray[ArrayIndex].normal = v3{ (float)Normal[0], (float)Normal[1], (float)Normal[2] };
					}

					// tangent data
					//u32 ElementTangentCount = pMesh->GetElementTangentCount();
					//if (ElementTangentCount == 0)
					//	pMesh->GenerateTangentsDataForAllUVSets();
					//for (u32 l = 0; l < ElementTangentCount; ++l)
					//{
					//	FbxVector4 Tangent;
					//	FbxGeometryElementTangent* lTangentElement = pMesh->GetElementTangent(l);
					//	if (lTangentElement)
					//	{
					//		if (lTangentElement->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
					//		{
					//			switch (lTangentElement->GetReferenceMode())
					//			{
					//			case FbxGeometryElement::eDirect:
					//			{
					//				Tangent = lTangentElement->GetDirectArray().GetAt(ArrayIndex); //?
					//			} break;

					//			case FbxGeometryElement::eIndexToDirect:
					//			{
					//				int id = lTangentElement->GetIndexArray().GetAt(ArrayIndex);
					//				Tangent = lTangentElement->GetDirectArray().GetAt(id);
					//			} break;

					//			default:
					//			{ }	break;
					//			}
					//		}
					//	}

					//	// storing tangent as float3s
					//	VertexArray[ArrayIndex].tx = (float)Tangent[0];
					//	VertexArray[ArrayIndex].ty = (float)Tangent[1];
					//	VertexArray[ArrayIndex].tz = (float)Tangent[2];
					//}

					ArrayIndex++;
				}
			}
		}

		for (u32 i = 0; i < (u32)VertexArray.size(); i += 3) // calculate tangents
		{
			assetTypes::vertex& v0 = VertexArray[i];
			assetTypes::vertex& v1 = VertexArray[i + 1];
			assetTypes::vertex& v2 = VertexArray[i + 2];

			assetTypes::v3 pos0 = v0.position;
			assetTypes::v3 pos1 = v1.position;
			assetTypes::v3 pos2 = v2.position;

			assetTypes::v2 uv0 = v0.uv;
			assetTypes::v2 uv1 = v1.uv;
			assetTypes::v2 uv2 = v2.uv;

			// Position delta
			assetTypes::v3 deltaPos1 = pos1 - pos0;
			assetTypes::v3 deltaPos2 = pos2 - pos0;

			// UV delta
			assetTypes::v2 deltaUV1 = uv1 - uv0;
			assetTypes::v2 deltaUV2 = uv2 - uv0;

			float r = 1.0f / (deltaUV1.u * deltaUV2.v - deltaUV1.v * deltaUV2.u);
			assetTypes::v3 tangent = (deltaPos1 * deltaUV2.v - deltaPos2 * deltaUV1.v) * r;
			assetTypes::v3 bitangent = (deltaPos2 * deltaUV1.u - deltaPos1 * deltaUV2.u) * r;

			v0.tangent = tangent;
			v1.tangent = tangent;
			v2.tangent = tangent;

			v0.bitangent = bitangent;
			v1.bitangent = bitangent;
			v2.bitangent = bitangent;
		}

		std::vector<u32> Indices;
		std::map<std::string, u32> IndexVertexMap;
		std::vector<assetTypes::vertex> FinalVertices;
		u32 CurrentIndex = 0;
		for (u32 i = 0; i < (u32)VertexArray.size(); i++)
		{
			if (IndexVertexMap.count((char*)&VertexArray[i]) == 0)
			{
				IndexVertexMap[(char*)&VertexArray[i]] = CurrentIndex;
				Indices.push_back(CurrentIndex++);
				FinalVertices.push_back(VertexArray[i]);
			}
			else
				Indices.push_back(IndexVertexMap[(char*)&VertexArray[i]]);
		}

		mesh_data MeshData;
		MeshData.NumVertices = (u32)FinalVertices.size();
		MeshData.NumIndices = (u32)Indices.size();

		u32 VertDataLength = MeshData.NumVertices * sizeof(assetTypes::vertex);
		u32 IndexDataLength = MeshData.NumIndices * sizeof(u32);
		u32 ExtraSize = sizeof(MeshData);

		// Set fields
		Out_ExtraData = new char[ExtraSize];
		memcpy(Out_ExtraData, &MeshData, ExtraSize);
		Out_RawData = new char[VertDataLength + IndexDataLength]; // Allocate new array since std::vector auto deletes data
		memcpy(Out_RawData, (char*)FinalVertices.data(), VertDataLength);
		memcpy(Out_RawData + VertDataLength, (char*)Indices.data(), IndexDataLength);
		Out_ExtraDataSize = ExtraSize;
		Out_RawDataSize = VertDataLength + IndexDataLength;
		
		return true;
	}
	else
		return false;
}

cAsset* assetTypes::Mesh_InitializeData(cAsset* AssetDefaults, char* ExtraData, u32 ExtraDataSize)
{
	mesh_data MeshData = *((mesh_data*)ExtraData);
	cMeshAsset* MeshAsset = new cMeshAsset();
	MeshAsset->CopyFields(AssetDefaults);

	MeshAsset->MeshData = MeshData;

	//MeshAsset->LoadAssetData();

	return MeshAsset;
}