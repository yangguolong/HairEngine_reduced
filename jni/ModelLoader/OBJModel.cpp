#include <stdio.h>
#include "OBJModel.h"
#include "glm.hpp"
#define BUF_SIZE 256

OBJModel::OBJModel()
{
	vVertexs.clear();
	vNormals.clear();
    vFaceNormals.clear();
	vTexcoords.clear();
	vGroups.clear();
	vMaterial.clear();
	//modelFileDir.clear();
}

OBJModel::~OBJModel()
{
	vVertexs.clear();
	vNormals.clear();
	vTexcoords.clear();
	vGroups.clear();
}

void OBJModel::resetOBJModel()
{
	//modelFileDir.clear();
	vVertexs.clear();
	vNormals.clear();
    vFaceNormals.clear();
	vTexcoords.clear();
	vMaterial.clear();
	for(int i=vGroups.size()-1;i>=0;i--)
	{
		for(int j = vGroups[i].vFaces.size() -1;j>=0;j--)
			vGroups[i].vFaces[j].vFaceVertexs.clear();
		vGroups[i].vFaces.clear();
	}	
	vGroups.clear();
}

void skipLine(char * buf, int size, FILE * fp)
{
	do {
		buf[size-1] = '$';
		fgets(buf, size, fp);
	} while (buf[size-1] != '$');
}

bool OBJModel::loadOBJFile(const char *filePath)
{
	FILE * fp = fopen(filePath,"r");
	if(!fp)
	{
		printf("Fail to open Obj file.\n");
		return false;
	}

	resetOBJModel();
	//modelFileDir.append(dirPath);
    BoundBox newBoundBox;

	char buf[BUF_SIZE];
	OBJGroup group;
	OBJFace face;
	OBJFaceVertex faceVertex;
	OBJVertex vertex;
	OBJNormal normal;
	OBJTexCoord texCoord;
	int faceVertexCount = 0;
	bool needReadNext = true;
	int currentMaterialID = -1;
	bool hasTexture = false;
	string matFilePath;
	string matFileName;
	//int index;
	while(true){

		if(needReadNext)
		{
			if(fscanf(fp,"%s",buf) == EOF)
				break;
		}
		else
			needReadNext = true;
		switch(buf[0]){
			case '#':
				//comment line, eat the remainder
				skipLine( buf, BUF_SIZE, fp);
                break;
			case 'm':
				if( strlen(buf) == 2 && buf[1] == 'g' )
					break;
				else if( 0 == strcmp(buf,"mtllib"))
				{

					fscanf(fp,"%s",buf);
//					matFilePath.clear();
//					matFilePath.append(dirPath);
//					matFilePath.append("/");
//
//					matFileName.clear();
//					matFileName.append(buf);
//					index = matFileName.find_last_of("/");
//					if(index >= 0)
//						matFileName.erase(0,index+1);
//					matFilePath.append(matFileName);
//
//					if( ! loadMaterial(matFilePath.c_str(),dirPath))
//					{
//						printf("Failed to load the material file.\n");
//						return false;
//					}
				}
				else
				{
					printf("Format error(v style): %s is not a legal keyword.\n",buf);
					return false;
				}
				break;
			case 'u':
				if( 0 == strcmp(buf,"usemtl"))
				{
					fscanf(fp,"%s",buf);
					currentMaterialID = getMaterialID(buf);
					if(currentMaterialID < 0 )
					{
						printf("Material %s can't match anyone in the material lib.\n",buf);
						return false;
					}
					if(vMaterial[currentMaterialID].textureFilePath.empty())
						hasTexture = false;
					else
						hasTexture = true;
				}
				else
				{
					printf("Format error(u style): %s is not a legal keyword.\n",buf);
					return false;
				}
				break;
			case 'v':
				switch(buf[1]){
					
					case '\0':
						if(fscanf(fp,"%f %f %f",vertex.position,vertex.position+1,vertex.position+2) != 3)
						{
							printf("vertex error : not 3 componments.\n");
							return false;
						}
						vVertexs.push_back(vertex);
						for(int i = 0;i <3;i++)
						{
							if(newBoundBox.boundMin[i] > vertex.position[i])
								newBoundBox.boundMin[i] = vertex.position[i];
							if(newBoundBox.boundMax[i] < vertex.position[i])
								newBoundBox.boundMax[i] = vertex.position[i];
						}
						break;
					case 'n':
						if(fscanf(fp,"%f %f %f",normal.normal,normal.normal+1,normal.normal+2) != 3)
						{
							printf("normal error : not 3 componments.\n");
							return false;
						}
						vNormals.push_back(normal);
						break;
					case 't':
						if(fscanf(fp,"%f %f",texCoord.uv,texCoord.uv+1) != 2)
						{
							printf("texcoord error : not 3 componments.\n");
							return false;
						}
						vTexcoords.push_back(texCoord);
						break;
					default :
						printf("Format error(v style): %s is not a legal keyword.\n",buf);
						return false;
				}
				break;
			case 'g':
				if(buf[1] != '\0'){
					printf("Format error(g style): %s is not a legal keyword.\n",buf);
					return false;
				}
				group.groupName.clear();
				group.vFaces.clear();
				fscanf(fp,"%s",buf);
				group.groupName.assign(buf);
				vGroups.push_back(group);
				break;
			case 'f':
				if(buf[1] != '\0'){
					printf("Format error(f style): %s is not a legal keyword.\n",buf);
					return false;
				}
				if(vGroups.empty())
					vGroups.push_back(OBJGroup("DefaultGroup"));
				vGroups.back().vFaces.push_back(OBJFace());
				vGroups.back().vFaces.back().materialID = currentMaterialID ;
				vGroups.back().vFaces.back().hasTexture = hasTexture;
				faceVertexCount = 0;
				fscanf(fp,"%s",buf);

				if(sscanf(buf,"%d/%d/%d",&faceVertex.vertexIndex,&faceVertex.uvIndex,&faceVertex.normalIndex) == 3)
				{
					faceVertex.vertexIndex--;
					faceVertex.uvIndex--;
					faceVertex.normalIndex--;
					vGroups.back().vFaces.back().vFaceVertexs.push_back(faceVertex);
					vGroups.back().vFaces.back().flag = FACE_HasNormal | FACE_HasTexcoord;
					faceVertexCount++;

					while(fscanf(fp,"%s",buf) != EOF)
					{
						if(sscanf(buf,"%d/%d/%d",&faceVertex.vertexIndex,&faceVertex.uvIndex,&faceVertex.normalIndex) ==3)
						{
							faceVertex.vertexIndex--;
							faceVertex.uvIndex--;
							faceVertex.normalIndex--;
							vGroups.back().vFaces.back().vFaceVertexs.push_back(faceVertex);
							faceVertexCount++;
						} else if( isalpha(buf[0] ) || (buf[0] == '#') )
						{
							//×Ö·û
							needReadNext = false;
							break;
						}else
						{
							printf("face vertex error(f style flag 1):\" %s \" can't be analyzed\n",buf);
							return false;
						}
					}

					if(faceVertexCount<3)
					{
						printf("face vertex error(f style flag 2):no enough face vertexs to define a face.\n");
						return false;
					}

				}else if(sscanf(buf,"%d/%d",&faceVertex.vertexIndex,&faceVertex.uvIndex) == 2)
				{
					faceVertex.vertexIndex--;
					faceVertex.uvIndex--;
					faceVertex.normalIndex = 0;
					vGroups.back().vFaces.back().vFaceVertexs.push_back(faceVertex);
					vGroups.back().vFaces.back().flag = FACE_HasTexcoord;
					faceVertexCount++;

					while(fscanf(fp,"%s",buf) != EOF)
					{
						if(sscanf(buf,"%d/%d",&faceVertex.vertexIndex,&faceVertex.uvIndex) == 2)
						{
							faceVertex.vertexIndex--;
							faceVertex.uvIndex--;
							faceVertex.normalIndex = 0;
							vGroups.back().vFaces.back().vFaceVertexs.push_back(faceVertex);
							faceVertexCount++;
						} else if( isalpha(buf[0] ) || (buf[0] == '#') )
						{
							//×Ö·û
							needReadNext = false;
							break;
						}else
						{
							printf("face vertex error(f style flag 3):\" %s \" can't be analyzed\n",buf);
							return false;
						}
					}

					if(faceVertexCount<3)
					{
						printf("face vertex error(f style flag 4):no enough face vertexs to define a face.\n");
						return false;
					}

				}else if(sscanf(buf,"%d//%d",&faceVertex.vertexIndex,&faceVertex.normalIndex) == 2)
				{
					faceVertex.vertexIndex--;
					faceVertex.uvIndex = 0;
					faceVertex.normalIndex--;
					vGroups.back().vFaces.back().vFaceVertexs.push_back(faceVertex);
					vGroups.back().vFaces.back().flag = FACE_HasNormal;
					faceVertexCount++;

					while(fscanf(fp,"%s",buf) != EOF)
					{
						if(sscanf(buf,"%d//%d",&faceVertex.vertexIndex,&faceVertex.normalIndex) == 2)
						{
							faceVertex.vertexIndex--;
							faceVertex.uvIndex = 0;
							faceVertex.normalIndex--;
							vGroups.back().vFaces.back().vFaceVertexs.push_back(faceVertex);
							faceVertexCount++;
						} else if( isalpha(buf[0] ) || (buf[0] == '#') )
						{
							//×Ö·û
							needReadNext = false;
							break;
						}else
						{
							printf("face vertex error(f style flag 5):\" %s \" can't be analyzed\n",buf);
							return false;
						}
					}

					if(faceVertexCount<3)
					{
						printf("face vertex error(f style flag 6):no enough face vertexs to define a face.\n");
						return false;
					}

				}else if(sscanf(buf,"%d",&faceVertex.vertexIndex) == 1)
				{

					faceVertex.vertexIndex--;
					faceVertex.uvIndex =0;
					faceVertex.normalIndex =0;
					vGroups.back().vFaces.back().vFaceVertexs.push_back(faceVertex);
					faceVertexCount++;

					while(fscanf(fp,"%s",buf) != EOF)
					{
						if(sscanf(buf,"%d",&faceVertex.vertexIndex) == 1)
						{
							faceVertex.vertexIndex--;
							faceVertex.uvIndex =0;
							faceVertex.normalIndex =0;
							vGroups.back().vFaces.back().vFaceVertexs.push_back(faceVertex);
							faceVertexCount++;
						} else if( isalpha(buf[0] ) || (buf[0] == '#') )
						{
							//×Ö·û
							needReadNext = false;
							break;
						}else
						{
							printf("face vertex error(f style flag 7):\" %s \" can't be analyzed\n",buf);
							return false;
						}
					}

					if(faceVertexCount<3)
					{
						printf("face vertex error(f style flag 8):no enough face vertexs to define a face.\n");
						return false;
					}

				}else
				{
					printf("face vertex error(f style flag 9):\" %s \" can't be analyzed\n",buf);
					return false;
				}
				break;
			default :
				break;
		}
	}

	for(int i = 0;i<3;i++)
		newBoundBox.boundCenter[i] = (newBoundBox.boundMax[i] + newBoundBox.boundMin[i])*0.5f; 
	newBoundBox.xLength = newBoundBox.boundMax[0] - newBoundBox.boundMin[0];
	newBoundBox.yLength = newBoundBox.boundMax[1] - newBoundBox.boundMin[1];
	newBoundBox.zLength = newBoundBox.boundMax[2] - newBoundBox.boundMin[2];
	boundBox = newBoundBox;
	//Unitize();
	
	return true;
}

void OBJModel::Unitize()
{
	float l = boundBox.boundMax[0] - boundBox.boundMin[0];
	float h = boundBox.boundMax[1] - boundBox.boundMin[1];
	float w = boundBox.boundMax[2] - boundBox.boundMin[2];

	float maxLen;
	//l>h?(l>w?maxLen = l:maxLen = w):(h>w?maxLen = h:maxLen = w);
	maxLen = l>h?(l>w? l:w):(h>w? h:w);
	float scale = 2.0f/maxLen;
	unsigned int size = vVertexs.size();
	for(unsigned int i = 0; i<size;i++)
	{
		vVertexs[i].position[0] = (vVertexs[i].position[0] - boundBox.boundCenter[0]) * scale;
		vVertexs[i].position[1] = (vVertexs[i].position[1] - boundBox.boundCenter[1]) * scale;
		vVertexs[i].position[2] = (vVertexs[i].position[2] - boundBox.boundCenter[2]) * scale;
	}
}

void OBJModel::evaluateFaceNormals()
{
    vFaceNormals.clear();
    
    int groupSize = vGroups.size();
    for(int i=0; i<groupSize; i++)
    {
        OBJGroup & group = vGroups[i];
        int faceSize = group.vFaces.size();
        for(int j=0; j<faceSize; j++)
        {
            OBJFace & face = group.vFaces[j];
            OBJVertex & vertex0 = vVertexs[face.vFaceVertexs[0].vertexIndex];
            OBJVertex & vertex1 = vVertexs[face.vFaceVertexs[1].vertexIndex];
            OBJVertex & vertex2 = vVertexs[face.vFaceVertexs[2].vertexIndex];
            
            glm::vec3 vec1 = glm::vec3(vertex1.position[0] - vertex0.position[0],
                                       vertex1.position[1] - vertex0.position[1],
                                       vertex1.position[2] - vertex0.position[2]);
            glm::vec3 vec2 = glm::vec3(vertex2.position[0] - vertex0.position[0],
                                       vertex2.position[1] - vertex0.position[1],
                                       vertex2.position[2] - vertex0.position[2]);
            glm::vec3 normalVec = glm::cross(vec1, vec2);
            normalVec = glm::normalize(normalVec);
            
            OBJNormal faceNormal;
            faceNormal.normal[0] = normalVec.x;
            faceNormal.normal[1] = normalVec.y;
            faceNormal.normal[2] = normalVec.z;
            vFaceNormals.push_back(faceNormal);
            //printf("%d  (%f %f %f)\n", (int)(vFaceNormals.size() - 1), faceNormal.normal[0], faceNormal.normal[1], faceNormal.normal[2]);
        }
    }
}

bool OBJModel::loadMaterial(const char * path,const char * dirPath)
{
	FILE * fp = fopen(path,"r");
	if(!fp){
		printf("Fail to open Material file.\n");
		return false;
	}

	vMaterial.clear();
	
	char buf[BUF_SIZE];
	OBJMaterial mat;

	while(fscanf(fp,"%s",buf) != EOF)
	{
		switch(buf[0])
		{
		case '#':
			skipLine( buf, BUF_SIZE, fp);
			break;	
		case 'k':
		case 'K':
			if(strlen(buf) == 2)
			{
				switch(buf[1])
				{
				case 'a':
				case 'A':
						fscanf(fp,"%f %f %f",mat.ka,mat.ka+1,mat.ka+2);
						break;
				case 'd':
				case 'D':
						fscanf(fp,"%f %f %f",mat.kd,mat.kd+1,mat.kd+2);
						break;
				case 's':
				case 'S':
						fscanf(fp,"%f %f %f",mat.ks,mat.ks+1,mat.ks+2);
						break;
				default:
					printf("Format error(Material-KX style): %s is not a legal keyword.\n",buf);
					return false;
				}
			}
			else
			{
				printf("Format error(Material-K style): %s is not a legal keyword.\n",buf);
				return false;
			}
			break;
		case 'd':
		case 'D':
			if( strlen(buf) == 1 )
			{
				fscanf(fp,"%f",&mat.d_Tr);
			}
			else
			{
				printf("Format error(Material-D style): %s is not a legal keyword.\n",buf);
				return false;
			}
			break;
		case 't':
		case 'T':
			if(strlen(buf) == 2 && (buf[1] == 'r' || buf[1] == 'R') )
			{
				fscanf(fp,"%f",&mat.d_Tr);
			}
			else
			{
				printf("Format error(Material-Tr style): %s is not a legal keyword.\n",buf);
				return false;
			}
			break;
		case 'n':
		case 'N':
			if(strlen(buf) == 6 && (buf[5] == 'l' || buf[5] =='L') )
			{
				if( ! mat.materialName.empty())
				{
					mat.materialID = vMaterial.size();
					vMaterial.push_back(mat);
				}
				mat.initMaterial();
				fscanf(fp,"%s",buf);
				mat.materialName.append(buf);
			}
			else if( strlen(buf) == 2 && (buf[1] == 's' || buf[1] == 'S' ) )
			{
				fscanf(fp,"%f",&mat.ns);
			}
			else
			{
				printf("Format error(Material-Ns style): %s is not a legal keyword.\n",buf);
				return false;
			}
			break;
		case 'i':
		case 'I':
			if( strlen(buf) == 5 && (buf[4] == 'm' || buf[4] == 'M' ) )
			{
				fscanf(fp,"%d",&mat.illum);
			}
			else
			{
				printf("Format error(Material-illum style): %s is not a legal keyword.\n",buf);
				return false;
			}
			break;
		case 'm':
		case 'M':
			if( strlen(buf) == 6 && (buf[5] == 'd' || buf[5] == 'D' ) )
			{
				fscanf(fp,"%s",buf);
				mat.textureFilePath.clear();
				mat.textureFilePath.append(dirPath);
				mat.textureFilePath.append("/");
				mat.textureFilePath.append(buf);
			}
			else
			{
				printf("Format error(Material-map_Kb  style): %s is not a legal keyword.\n",buf);
				return false;
			}
			break;
		default:
			break;
		}
	}
	
	if( ! mat.materialName.empty())
	{
		mat.materialID = vMaterial.size();
		vMaterial.push_back(mat);
	}
	return true;
}

int OBJModel::getMaterialID(char * matName)
{
	int size = vMaterial.size();
	for(int i = 0; i < size;i++)
	{
		if(0 == strcmp(vMaterial[i].materialName.c_str(),matName))
			return i;
	}
	return -1;
}