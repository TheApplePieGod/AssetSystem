#include "pch.h"
#include "../assetLoader.h"
#include "../assetLoader_internal.h"

///*
//* Returns new file path
//*/
//std::string assetLoader::PackMesh(std::string Path, std::string Filename, int AssetID, std::ofstream* ofs, bool GeneratePac)
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
//	if (!lImporter->Initialize(Path.c_str(), -1, lSdkManager->GetIOSettings())) {
//		printf("Call to FbxImporter::Initialize() failed.\n");
//		printf("Error returned: %s\n\n", lImporter->GetStatus().GetErrorString());
//		return "NULL";
//	}
//
//	// Create a new scene so that it can be populated by the imported file.
//	FbxScene* lScene = FbxScene::Create(lSdkManager, "myScene");
//
//	// Import the contents of the file into the scene.
//	if (!lImporter->Import(lScene))
//		return "NULL";
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
//		std::vector<mesh_vertex> VertexArray;
//		u32 ArrayIndex = 0;
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
//			FbxMesh* pMesh = (FbxMesh*)pFbxChildNode->GetNodeAttribute();
//
//			FbxVector4* lControlPoints = pMesh->GetControlPoints();
//
//			u32 countd = pMesh->GetPolygonCount();
//			for (s32 j = 0; j < pMesh->GetPolygonCount(); j++)
//			{
//				u32 NumVertices = pMesh->GetPolygonSize(j);
//				if (NumVertices != 3) // must be triangulated
//					return "NULL";
//
//				for (u32 k = 0; k < NumVertices; k++)
//					//for (s32 k = NumVertices - 1; k >= 0; k--)
//				{
//					mesh_vertex vert = mesh_vertex();
//					VertexArray.push_back(vert);
//
//					//vertices data
//					u32 ControlPointIndex = pMesh->GetPolygonVertex(j, k);
//					FbxVector4 Vertex = lControlPoints[ControlPointIndex];
//
//					VertexArray[ArrayIndex].x = (float)Vertex[0];
//					VertexArray[ArrayIndex].y = (float)Vertex[1];
//					VertexArray[ArrayIndex].z = (float)Vertex[2];
//
//					//UV data
//					u32 ElementUVCount = pMesh->GetElementUVCount();
//					for (u32 l = 0; l < ElementUVCount; ++l)
//					{
//						FbxGeometryElementUV* leUV = pMesh->GetElementUV(l);
//						FbxVector2 UV;
//						switch (leUV->GetMappingMode())
//						{
//						default:
//						{ }	break;
//
//						case FbxGeometryElement::eByControlPoint:
//						{
//							switch (leUV->GetReferenceMode())
//							{
//							case FbxGeometryElement::eDirect:
//							{
//								UV = leUV->GetDirectArray().GetAt(ControlPointIndex);
//							} break;
//
//							case FbxGeometryElement::eIndexToDirect:
//							{
//								int id = leUV->GetIndexArray().GetAt(ControlPointIndex);
//								UV = leUV->GetDirectArray().GetAt(id);
//							} break;
//
//							default:
//							{ }	break; // other reference modes not shown here!
//							}
//						} break;
//
//						case FbxGeometryElement::eByPolygonVertex:
//						{
//							int lTextureUVIndex = pMesh->GetTextureUVIndex(j, k);
//							switch (leUV->GetReferenceMode())
//							{
//							case FbxGeometryElement::eDirect:
//							case FbxGeometryElement::eIndexToDirect:
//							{
//								UV = leUV->GetDirectArray().GetAt(lTextureUVIndex);
//							} break;
//
//							default:
//							{ }	break; // other reference modes not shown here!
//							}
//						} break;
//
//						case FbxGeometryElement::eByPolygon: // doesn't make much sense for UVs
//						case FbxGeometryElement::eAllSame:   // doesn't make much sense for UVs
//						case FbxGeometryElement::eNone:       // doesn't make much sense for UVs
//						{ }	break;
//						}
//
//						VertexArray[ArrayIndex].u = (float)UV[0];
//						VertexArray[ArrayIndex].v = (float)UV[1]; // Invert V due to difference in coord systems between Maya and D3D
//					}
//
//					// normal data
//					u32 ElementNormalCount = pMesh->GetElementNormalCount();
//					if (ElementNormalCount == 0)
//						pMesh->GenerateNormals();
//					for (u32 l = 0; l < ElementNormalCount; ++l)
//					{
//						FbxVector4 Normal;
//						FbxGeometryElementNormal* lNormalElement = pMesh->GetElementNormal(l);
//						if (lNormalElement)
//						{
//							if (lNormalElement->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
//							{
//								switch (lNormalElement->GetReferenceMode())
//								{
//								case FbxGeometryElement::eDirect:
//								{
//									Normal = lNormalElement->GetDirectArray().GetAt(ArrayIndex);
//								} break;
//
//								case FbxGeometryElement::eIndexToDirect:
//								{
//									int id = lNormalElement->GetIndexArray().GetAt(ArrayIndex);
//									Normal = lNormalElement->GetDirectArray().GetAt(id);
//								} break;
//
//								default:
//								{ }	break;
//								}
//							}
//						}
//
//						// storing normals as float3s
//						VertexArray[ArrayIndex].nx = (float)Normal[0];
//						VertexArray[ArrayIndex].ny = (float)Normal[1];
//						VertexArray[ArrayIndex].nz = (float)Normal[2];
//					}
//
//					// tangent data
//					u32 ElementTangentCount = pMesh->GetElementTangentCount();
//					if (ElementTangentCount == 0)
//						pMesh->GenerateTangentsDataForAllUVSets();
//					for (u32 l = 0; l < ElementTangentCount; ++l)
//					{
//						FbxVector4 Tangent;
//						FbxGeometryElementTangent* lTangentElement = pMesh->GetElementTangent(l);
//						if (lTangentElement)
//						{
//							if (lTangentElement->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
//							{
//								switch (lTangentElement->GetReferenceMode())
//								{
//								case FbxGeometryElement::eDirect:
//								{
//									Tangent = lTangentElement->GetDirectArray().GetAt(ArrayIndex); //?
//								} break;
//
//								case FbxGeometryElement::eIndexToDirect:
//								{
//									int id = lTangentElement->GetIndexArray().GetAt(ArrayIndex);
//									Tangent = lTangentElement->GetDirectArray().GetAt(id);
//								} break;
//
//								default:
//								{ }	break;
//								}
//							}
//						}
//
//						// storing tangent as float3s
//						VertexArray[ArrayIndex].tx = (float)Tangent[0];
//						VertexArray[ArrayIndex].ty = (float)Tangent[1];
//						VertexArray[ArrayIndex].tz = (float)Tangent[2];
//					}
//
//					ArrayIndex++;
//				}
//			}
//		}
//
//		mesh_pack MeshPack;
//		MeshPack.NumVertices = (u32)VertexArray.size();
//
//		asset_header Header;
//		Header.ID = AssetID;
//		Header.Type = asset_type::Mesh;
//		Header.DataSize = MeshPack.NumVertices * sizeof(mesh_vertex);
//		Header.ExtraSize = 0;
//		Header.NextItem = sizeof(mesh_pack) + Header.DataSize;
//		strcpy(Header.Filename, Filename.c_str());
//
//		if (ofs != nullptr)
//		{
//			if (GeneratePac)
//			{
//				ofs->write((char*)&Header, sizeof(asset_header));
//				ofs->write((char*)&MeshPack, sizeof(mesh_pack));
//				ofs->write((char*)VertexArray.data(), Header.DataSize);
//			}
//		}
//
//		remove(Path.c_str());
//		std::string NewPath = Path;
//		NewPath.erase(NewPath.length() - 4, 4);
//		NewPath.push_back('.');
//		NewPath += GetAssetSettings().AssetFileExtension;
//		FILE* file;
//		file = fopen(NewPath.c_str(), "wb");
//		if (file)
//		{
//			fwrite((char*)&Header, sizeof(asset_header), 1, file);
//			fwrite((char*)&MeshPack, sizeof(mesh_pack), 1, file);
//			fwrite((char*)VertexArray.data(), sizeof(char), Header.DataSize, file);
//			fclose(file);
//		}
//
//		return NewPath;
//	}
//	else
//		return "NULL";
//}
//
//const char* assetLoader::PackMesh(const char* Path, int AssetID)
//{
//	WIN32_FIND_DATA data;
//	HANDLE hFind = FindFirstFile(Path, &data);
//
//	std::string ret = assetLoader::PackMesh(Path, data.cFileName, AssetID);
//	char* retptr = new char[ret.size() + 1];
//	strcpy(retptr, ret.c_str());
//
//	return retptr;
//}
//
//void assetLoader::LoadMesh(FILE* File, asset_header& Header, std::string Path, void (*Callback)(cMeshAsset*))
//{
//	mesh_pack reading;
//	size_t d = fread((char*)&reading, sizeof(reading), 1, File);
//	cMeshAsset* MeshAsset = new cMeshAsset;
//	MeshAsset->AssetID = Header.ID;
//	MeshAsset->Type = asset_type::Mesh;
//	MeshAsset->DataSize = Header.DataSize;
//	MeshAsset->VertexCount = reading.NumVertices;
//	strncpy(MeshAsset->Filename, Header.Filename, MAX_PATH);
//	strncpy(MeshAsset->Path, Path.c_str(), MAX_PATH);
//	MeshAsset->LoadAssetData();
//
//	(*Callback)(MeshAsset);
//}
//
//void assetLoader::LoadMesh(const char* Path, void (*Callback)(cMeshAsset*))
//{
//	FILE* file = fopen(Path, "rb");
//	if (file)
//	{
//		asset_header Header;
//		fread((char*)&Header, 1, sizeof(asset_header), file);
//		LoadMesh(file, Header, Path, Callback);
//
//		fclose(file);
//	}
//}