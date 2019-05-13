#ifndef OBJMODEL_H
#define OBJMODEL_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include "ModelCommonData.h"
using  namespace std;

struct OBJVertex
{
	float position[3];
};

struct OBJNormal
{
	float normal[3];
};

struct OBJTexCoord
{
	float uv[2];
};

struct OBJFaceVertex
{
	OBJFaceVertex()
	{
		vertexIndex = 0;
		normalIndex = 0;
		uvIndex = 0;
	}
	unsigned int vertexIndex;
	unsigned int normalIndex;
	unsigned int uvIndex;
};

struct OBJMaterial
{
	OBJMaterial()
	{
		initMaterial();
	}
	void initMaterial()
	{
		materialName.clear();
		materialID = -1;
		for(int i = 0;i<3;i++)
		{
			ka[i] = 0.2f;
			kd[i] = 0.8f;
			ks[i] = 1.0f;
		}
		d_Tr = 1;
		ns = 0.0f;
		illum = 1;
		textureFilePath.clear();
	}
	string materialName;
	int materialID ;
	float ka[3];
	float kd[3];
	float ks[3];
	float d_Tr;
	float ns;
	int illum;
	string textureFilePath;
};

#define FACE_HasNormal 0x01
#define FACE_HasTexcoord 0x10

struct OBJFace
{
	OBJFace()
	{
		flag = 0x00;
		vFaceVertexs.clear();
		materialID = -1;
		hasTexture = false;
	}
	unsigned char flag;
	vector<OBJFaceVertex> vFaceVertexs;
	int materialID;
	bool hasTexture;
};

struct OBJGroup
{
	OBJGroup()
	{
		groupName.clear();
		vFaces.clear();
	}
	OBJGroup(string name)
	{
		groupName = name;
	}
	string groupName;
	vector<OBJFace> vFaces;
};

class OBJModel
{
public:
		OBJModel();
		~OBJModel();
	
		//bool loadOBJFile(const char * filePath, const char * dirPath);
        bool loadOBJFile(const char * filePath); //忽略Material 的读取
		void Unitize();
        void evaluateFaceNormals();
		void resetOBJModel();
public:
		vector<OBJVertex> vVertexs;
		vector<OBJNormal> vNormals;
        vector<OBJNormal> vFaceNormals; //evaluate by OBJModel
		vector<OBJTexCoord> vTexcoords;
		vector<OBJMaterial> vMaterial;
		vector<OBJGroup> vGroups;
		BoundBox boundBox;
		//string modelFileDir;
private:
		
		bool loadMaterial(const char * path, const char * dirPath);
		int getMaterialID(char * matName);
};

#endif