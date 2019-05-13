#include "HeadGenerator.h"
#include "HalfFloat.h"

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <map>
#include <stack>

#include <android/log.h>
#define LOG_TAG "HeadGenerator"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)


using namespace cv;

#define FeatureSize			76
#define BoundaryFeatureSize	15
#define VertexSize			5832
#define InnerBoundarySize	104
#define FaceSize			5746
#define ComponentSize		40//1109
#define TrainSize			40

static int leftEyeHoleVertex[28] = {
	1479, 1482, 1636, 1457,
	1456, 1459, 3104, 3105,
	3136, 3138, 3788, 3157,
	3155, 3156, 3216, 2614,
	2613, 2616, 2674, 2672,
	2671, 1629, 1627, 1631,
	2676, 1813, 1812, 1480
};

static int leftEyeHoleTexture[28] = {
	1544, 1547, 1704, 1522,
	1521, 1524, 3209, 3210,
	3241, 3243, 3903, 3262,
	3260, 3261, 3322, 2700,
	2699, 2702, 2760, 2758,
	2757, 1697, 1695, 1699,
	2762, 1887, 1886, 1545
};

static int rightEyeHoleVertex[28] = {
	1875, 1879, 2046, 1854,
	1852, 1856, 4868, 4870,
	4900, 4901, 5539, 4921,
	4919, 4922, 4978, 4415,
	4412, 4413, 4473, 4472,
	4470, 1883, 1880, 1881,
	4475, 2220, 2218, 1877
};

static int rightEyeHoleTexture[28] = {
	1950, 1954, 2125, 1929,
	1927, 1931, 5025, 5027,
	5057, 5058, 5705, 5078,
	5076, 5079, 5136, 4553,
	4550, 4551, 4611, 4610,
	4608, 1958, 1955, 1956,
	4613, 2305, 2303, 1952
};

static int mouthHoleVertex[44] = {
	1885, 1886, 4506, 4504,
	4502, 4503, 4599, 4601,
	5810, 4843, 4841, 3073,
	3072, 3074, 4064, 2811,
	2810, 2707, 2704, 2706,
	2708, 1485, 1484, 1487,
	4063, 3626, 3625, 3628,
	3649, 2826, 2825, 2795,
	2794, 2796, 4585, 4586,
	4612, 4613, 5405, 5382,
	5381, 5384, 5809, 1887
};

static int mouthHoleTexture[44] = {
	1960, 1961, 4644, 4642,
	4640, 4641, 4737, 4739,
	5999, 5000, 4998, 3178,
	3177, 3179, 4201, 2897,
	2896, 2793, 2790, 2792,
	2794, 1550, 1549, 1552,
	4200, 3733, 3732, 3735,
	3756, 2912, 2911, 2881,
	2880, 2882, 4723, 4724,
	4750, 4751, 5564, 5541,
	5540, 5543, 5998, 1962
};

HeadGenerator::HeadGenerator(const char * featureFile, const char * trainResultFile, const char * auxiliaryDataFile, const char * foreheadVertexIndexFile)
{
	m_estimateIteration = 4;
	m_projectionIteration = 2;
	m_coefEnergy = 100.f;
	m_estimateDone = 0;
	m_activeFeatureNumber = 0;
	m_boundaryFaceNumber = 0;
	m_interiorFaceNumber = 0;
	m_boundaryVertexNumber = 0;
	m_activeFeatureNumber = FeatureSize;
	m_defaultFeatureIndexArray	= Mat::zeros(FeatureSize, 1, CV_32SC2);
	m_fittingFeatureIndexArray	= Mat::zeros(FeatureSize, 1, CV_32SC2);
	m_featureWeightArray		= Mat::zeros(FeatureSize, 1, CV_32FC1);
	m_featureActiveArray		= Mat::zeros(FeatureSize, 1, CV_8SC1);
	m_averageHeadArray			= Mat::zeros(VertexSize, 1, CV_32FC3);
	m_headFaceNeighborArray		= Mat::zeros(FaceSize, 1, CV_32SC4);
	m_headFaceIndexArray		= Mat::zeros(FaceSize, 1, CV_32SC4);
	m_headFaceBoundaryArray		= Mat::zeros(FaceSize, 1, CV_8SC1);
	m_headFaceNormalFlagArray	= Mat::zeros(FaceSize, 1, CV_8UC1);
	m_boundaryIndexArray		= Mat::zeros(VertexSize, 1, CV_32SC1);
	m_basisHeadValueArray		= Mat::zeros(TrainSize, 1, CV_32FC1);
	m_basisHeadVectorArray		= Mat::zeros(TrainSize, VertexSize, CV_32FC3);
	m_basisCoefficientArray		= Mat::zeros(TrainSize, VertexSize, CV_32FC1);
	m_basisLambdaMatInv			= Mat::zeros(TrainSize, TrainSize, CV_32FC1);
	m_fittingHeadArray			= Mat::zeros(VertexSize, 1, CV_32FC3);
	m_projectFittingHeadArray	= Mat::zeros(VertexSize, 1, CV_32FC3);
	m_modelFeatureArray			= Mat::zeros(FeatureSize, 1, CV_32FC3);
	m_projectModelFeatureArray	= Mat::zeros(FeatureSize, 1, CV_32FC3);
	m_imageFeatureArray			= Mat::zeros(FeatureSize, 1, CV_32FC2);

	FILE * file;
	// Feature indices file
	file = fopen(featureFile, "r");
	for (int featureI = 0, activeFeatureI = 0; featureI < FeatureSize; featureI++){
		int featureID;
		Vec2i vertexID;
		float featureWeight;
		fscanf(file, "%d %d %d %f", &featureID, &vertexID[0], &vertexID[1], &featureWeight);
		if (vertexID[0] < 0 || vertexID[1] < 0){
			m_featureActiveArray.at<char>(featureI) = 0;
			m_activeFeatureNumber--;
		}
		else{
			m_featureActiveArray.at<char>(featureI) = 1;
			m_defaultFeatureIndexArray.at<Vec2i>(activeFeatureI, 0) = vertexID;
			m_featureWeightArray.at<float>(activeFeatureI) = featureWeight;
			activeFeatureI++;
		}
	}
	fclose(file);

	// PCA training file
	m_resultFeatureArray	= Mat::zeros(m_activeFeatureNumber, 1, CV_32FC2);
	m_boundaryVertexArray	= Mat::zeros(VertexSize, 1, CV_32FC2);

//  load full float trainData (2.9M)
//	file = fopen(trainResultFile, "rb");
//	int vertexNum;
//	int componentNum;
//	fread(&vertexNum, sizeof(int), 1, file);
//	fread(&componentNum, sizeof(int), 1, file);
//	assert(vertexNum == VertexSize && componentNum == ComponentSize);
//    
//	fread(m_averageHeadArray.data, sizeof(Vec3f), VertexSize, file);
//	fread(m_basisHeadValueArray.data, sizeof(float), TrainSize, file);
//	fseek(file, sizeof(float) * (ComponentSize - TrainSize), SEEK_CUR);
//	fread(m_basisHeadVectorArray.data, sizeof(Vec3f), TrainSize * VertexSize, file);
//	fclose(file);
    
//  load half float trainData (1.4M)
    file = fopen(trainResultFile, "rb");
	int vertexNum;
	int componentNum;
	fread(&vertexNum, sizeof(int), 1, file);
	fread(&componentNum, sizeof(int), 1, file);
	assert(vertexNum == VertexSize && componentNum == ComponentSize);
    
    short * tmpAverageHeadArray = new short[VertexSize * 3];
    short * tmpBasisHeadValueArray = new short[TrainSize * 1];
    short * tmpBasisHeadVectorArray = new short[TrainSize * VertexSize * 3];
    
    fread(tmpAverageHeadArray, sizeof(short), VertexSize * 3, file);
    fread(tmpBasisHeadValueArray, sizeof(short), TrainSize * 1, file);
    fread(tmpBasisHeadVectorArray, sizeof(short), TrainSize * VertexSize * 3, file);
    fclose(file);
    
    halfp2singles(m_averageHeadArray.data, tmpAverageHeadArray, VertexSize * 3);
    halfp2singles(m_basisHeadValueArray.data, tmpBasisHeadValueArray, TrainSize * 1);
    halfp2singles(m_basisHeadVectorArray.data, tmpBasisHeadVectorArray, TrainSize * VertexSize * 3);
    
    delete [] tmpAverageHeadArray;
    delete [] tmpBasisHeadValueArray;
    delete [] tmpBasisHeadVectorArray;

	// Auxiliary file.
	file = fopen(auxiliaryDataFile, "r");
	for (int faceI = 0; faceI < FaceSize; faceI++){
		Vec4i neighborIndices;
		fscanf(file, "%d %d %d %d", &neighborIndices[0],  &neighborIndices[1],  &neighborIndices[2],  &neighborIndices[3]);
		m_headFaceNeighborArray.at<Vec4i>(faceI, 0) = neighborIndices;
	}
	for (int faceI = 0; faceI < FaceSize; faceI++){
		Vec4i vertexIndices;
		fscanf(file, "%d %d %d %d", &vertexIndices[0],  &vertexIndices[1],  &vertexIndices[2],  &vertexIndices[3]);
		m_headFaceIndexArray.at<Vec4i>(faceI, 0) = vertexIndices;
	}
	memset(m_headFaceBoundaryArray.data, 0, sizeof(char) * FaceSize);
	fscanf(file, "%d", &m_boundaryFaceNumber);
	for (int faceI = 0; faceI < m_boundaryFaceNumber; faceI++){
		int faceIndex;
		fscanf(file, "%d", &faceIndex);
		m_headFaceBoundaryArray.at<char>(faceIndex, 0) = 1;
	}
	fscanf(file, "%d",& m_interiorFaceNumber);
	for (int faceI = 0; faceI < m_interiorFaceNumber; faceI++){
		int faceIndex;
		fscanf(file, "%d", &faceIndex);
		m_headFaceBoundaryArray.at<char>(faceIndex, 0) = -1;
	}
	fclose(file);
    
    //forehead vertex index file.
    file = fopen(foreheadVertexIndexFile, "r");
    int foreheadVertexNum;
    fscanf(file, "%d", &foreheadVertexNum);
    m_foreheadVertexIndexVec.resize(foreheadVertexNum);
    for(int i=0; i<foreheadVertexNum; i++)
        fscanf(file, "%d", &m_foreheadVertexIndexVec[i]);
    fclose(file);

	reset();

	for (int trainI = 0; trainI < TrainSize; trainI++)
		m_basisLambdaMatInv.at<float>(trainI, trainI) =
			1.f / (m_basisHeadValueArray.at<float>(trainI, 0) * m_basisHeadValueArray.at<float>(trainI, 0));
}

HeadGenerator::~HeadGenerator()
{
}

void HeadGenerator::reset()
{
	m_estimateDone = 0;
	m_transform = Transform();
	memset(m_basisCoefficientArray.data, 0, sizeof(float) * TrainSize);
	m_fittingFeatureIndexArray = m_defaultFeatureIndexArray.clone();
	updateModel();
	updateFeature();
}

void HeadGenerator::save(const char * faceTexAndTopoFile, const char* boundPath, const char *coefPath, const char *modelPath)
{
	if (!m_estimateDone)
		return;
	Transform saveTransform;
	saveTransform.setRot(m_transform.getRot());
	if (boundPath){

		FILE * file = fopen(boundPath, "w");
		fprintf(file, "%d\n", m_boundaryVertexNumber);
		for (int vertexI = 0; vertexI < m_boundaryVertexNumber; vertexI++){
			Vec2f vertexPos = m_boundaryVertexArray.at<Vec2f>(vertexI, 0);
			int vertexInd = m_boundaryIndexArray.at<int>(vertexI, 0);
			fprintf(file, "%f %f %d\n", vertexPos[0], vertexPos[1], vertexInd);
		}
		fprintf(file, "15\n");
		for (int vertexI = 0; vertexI < 15; vertexI++){
			Vec2f boundaryPoint = m_imageFeatureArray.at<Vec2f>(vertexI, 0);
			fprintf(file, "%f %f\n",  boundaryPoint[0], m_imageHeight - boundaryPoint[1]);
		}
		fclose(file);
	}
	if (coefPath)
		m_transform.save(coefPath);
	if (modelPath){

		FILE * file = fopen(modelPath, "w");
		fprintf(file, "mtllib fit.mtl\ng fit\ns 1\n");
		for (int vertexI = 0; vertexI < VertexSize; vertexI++){
			Vec3f projectPoint = saveTransform * m_fittingHeadArray.at<Vec3f>(vertexI, 0);
			fprintf(file, "v %f %f %f\n", projectPoint[0], projectPoint[1], projectPoint[2]);
		}
		FILE * tmpFile = fopen(faceTexAndTopoFile, "r");
		int tmpChar;
		while((tmpChar = fgetc(tmpFile)) != EOF)
			fprintf(file, "%c", (char)tmpChar);
		fclose(tmpFile);

		// fill hole
		int beginIndex1;
		int beginIndex2;
		beginIndex1 = 0;
		beginIndex2 = 27;
		for (int holeI = 0; holeI < 13; holeI++){
			fprintf(file, "f %d/%d %d/%d %d/%d\n",
                    leftEyeHoleVertex[beginIndex1], leftEyeHoleTexture[beginIndex1],
                    leftEyeHoleVertex[beginIndex2], leftEyeHoleTexture[beginIndex2],
                    leftEyeHoleVertex[beginIndex2 - 1], leftEyeHoleTexture[beginIndex2 - 1]);
            fprintf(file, "f %d/%d %d/%d %d/%d\n",
                    leftEyeHoleVertex[beginIndex1], leftEyeHoleTexture[beginIndex1],
                    leftEyeHoleVertex[beginIndex2 - 1], leftEyeHoleTexture[beginIndex2 - 1],
                    leftEyeHoleVertex[beginIndex1 + 1], leftEyeHoleTexture[beginIndex1 + 1]);
			beginIndex1++;
			beginIndex2--;
		}
		beginIndex1 = 0;
		beginIndex2 = 27;
		for (int holeI = 0; holeI < 13; holeI++){
			fprintf(file, "f %d/%d %d/%d %d/%d\n",
                    rightEyeHoleVertex[beginIndex1], rightEyeHoleTexture[beginIndex1],
                    rightEyeHoleVertex[beginIndex1 + 1], rightEyeHoleTexture[beginIndex1 + 1],
                    rightEyeHoleVertex[beginIndex2 - 1], rightEyeHoleTexture[beginIndex2 - 1]);
            
            fprintf(file, "f %d/%d %d/%d %d/%d\n",
                    rightEyeHoleVertex[beginIndex1], rightEyeHoleTexture[beginIndex1],
                    rightEyeHoleVertex[beginIndex2 - 1], rightEyeHoleTexture[beginIndex2 - 1],
                    rightEyeHoleVertex[beginIndex2], rightEyeHoleTexture[beginIndex2]);
			beginIndex1++;
			beginIndex2--;
		}
		beginIndex1 = 0;
		beginIndex2 = 43;
		for(int holeI = 0; holeI < 21; holeI++){
			fprintf(file, "f %d/%d %d/%d %d/%d\n",
                    mouthHoleVertex[beginIndex1], mouthHoleTexture[beginIndex1],
                    mouthHoleVertex[beginIndex2], mouthHoleTexture[beginIndex2],
                    mouthHoleVertex[beginIndex2 - 1], mouthHoleTexture[beginIndex2 - 1]);
            
            fprintf(file, "f %d/%d %d/%d %d/%d\n",
                    mouthHoleVertex[beginIndex1], mouthHoleTexture[beginIndex1],
                    mouthHoleVertex[beginIndex2 - 1], mouthHoleTexture[beginIndex2 - 1],
                    mouthHoleVertex[beginIndex1 + 1], mouthHoleTexture[beginIndex1 + 1]);
			beginIndex1++;
			beginIndex2--;
		}
		fclose(file);
	}
}

void HeadGenerator::setFeatures(Mat *features, int width, int height)
{
	m_estimateDone = 0;
	m_imageWidth = width;
	m_imageHeight = height;
	for (int featureI = 0, activeFeatureI = 0; featureI < FeatureSize; featureI++)
		if (m_featureActiveArray.at<char>(featureI)){
			Vec2f featurePos = features->at<Vec2f>(0, featureI);
			featurePos[1] = m_imageHeight - featurePos[1];
			m_imageFeatureArray.at<Vec2f>(activeFeatureI++, 0) = featurePos;
		}
}

void HeadGenerator::runHeadGenerate()
{
	pojectionEstimate(true);
	updateModel();
	updateFeature();
	updateNormal();
	calculateBoundary();
	updateBoundary();
	for (int estimateI = 0; estimateI < m_estimateIteration; estimateI++){
		for (int projectI = 0; projectI < m_projectionIteration; projectI++){
			pojectionEstimate(false);
			updateModel();
			updateNormal();
			calculateBoundary();
			updateBoundary();
			updateFeature();
		}
		resetModel();
		updateFeature();
		coefficientEstimate();
		updateModel();
		updateNormal();
		calculateBoundary();
		updateBoundary();
		updateFeature();
	}
	m_estimateDone = 1;
	prepareResultArrays();
}

void HeadGenerator::resetModel()
{
	m_fittingHeadArray = m_averageHeadArray.clone();

	for (int vertexI = 0; vertexI < VertexSize; vertexI++)
		m_projectFittingHeadArray.at<Vec3f>(vertexI, 0) = m_transform * m_fittingHeadArray.at<Vec3f>(vertexI, 0);
}

void HeadGenerator::updateModel()
{
	m_fittingHeadArray = m_averageHeadArray.clone();

	for (int vertexI = 0; vertexI < VertexSize; vertexI++){
		for (int trainI = 0; trainI < TrainSize; trainI++)
			m_fittingHeadArray.at<Vec3f>(vertexI, 0) += m_basisCoefficientArray.at<float>(trainI, 0) * m_basisHeadVectorArray.at<Vec3f>(trainI, vertexI);
	}

	for (int vertexI = 0; vertexI < VertexSize; vertexI++)
		m_projectFittingHeadArray.at<Vec3f>(vertexI, 0) = m_transform * m_fittingHeadArray.at<Vec3f>(vertexI, 0);
}

void HeadGenerator::updateBoundary()
{
	Vec2f center = (m_imageFeatureArray.at<Vec2f>(0, 0) + m_imageFeatureArray.at<Vec2f>(14, 0)) * 0.5f;

	for (int featureI = 0; featureI < BoundaryFeatureSize; featureI++){
		int beginIndex = (featureI == 0) ? featureI : featureI - 1;
		int endIndex = (featureI == BoundaryFeatureSize - 1) ? featureI : featureI + 1;
		Vec2f featurePos = m_imageFeatureArray.at<Vec2f>(featureI, 0);
		Vec2f featureDir;
		featureDir[0] = m_imageFeatureArray.at<Vec2f>(endIndex, 0)[1] - m_imageFeatureArray.at<Vec2f>(beginIndex, 0)[1];
		featureDir[1] = m_imageFeatureArray.at<Vec2f>(beginIndex, 0)[0] - m_imageFeatureArray.at<Vec2f>(endIndex, 0)[0];
		if (featureDir.dot(featurePos - center) < 0.f)
			featureDir = -featureDir;
		int boundaryI;
		for (boundaryI = 0; boundaryI < m_boundaryVertexNumber; boundaryI++){
			int beginVertex = m_boundaryIndexArray.at<int>(boundaryI, 0);
			int endVertex = m_boundaryIndexArray.at<int>((boundaryI + 1) % m_boundaryVertexNumber, 0);
			Vec2f beginPos = Vec2f(m_projectFittingHeadArray.at<Vec3f>(beginVertex, 0).val);
			Vec2f endPos = Vec2f(m_projectFittingHeadArray.at<Vec3f>(endVertex, 0).val);
			if (featureDir.dot((beginPos + endPos) * 0.5f - center) < 0.f)
				continue;
			beginPos -= featurePos;
			endPos -= featurePos;
			float beginCross = featureDir[0] * beginPos[1] - featureDir[1] * beginPos[0];
			float endCross = featureDir[0] * endPos[1] - featureDir[1] * endPos[0];
			if (beginCross * endCross <= 0.f){
				m_fittingFeatureIndexArray.at<Vec2i>(featureI, 0) = Vec2i(beginVertex, endVertex);
				break;
			}
		}
		if (boundaryI == m_boundaryVertexNumber)
			printf("Error : no boundary fittiing %d.\n", featureI);
	}
}

void HeadGenerator::updateFeature()
{

	for (int featureI = 0; featureI < m_activeFeatureNumber; featureI++){
		Vec2i featureIndex = m_fittingFeatureIndexArray.at<Vec2i>(featureI, 0);
		m_modelFeatureArray.at<Vec3f>(featureI, 0) =
			(m_fittingHeadArray.at<Vec3f>(featureIndex[0], 0) + m_fittingHeadArray.at<Vec3f>(featureIndex[1], 0)) * 0.5f;
		m_projectModelFeatureArray.at<Vec3f>(featureI, 0) =
			(m_projectFittingHeadArray.at<Vec3f>(featureIndex[0], 0) + m_projectFittingHeadArray.at<Vec3f>(featureIndex[1], 0)) * 0.5f;
	}
}

void HeadGenerator::updateNormal()
{

	for (int faceI = 0; faceI < FaceSize; faceI++){
		Vec3f dirAB =
			m_projectFittingHeadArray.at<Vec3f>(m_headFaceIndexArray.at<Vec4i>(faceI, 0)[1], 0) -
			m_projectFittingHeadArray.at<Vec3f>(m_headFaceIndexArray.at<Vec4i>(faceI, 0)[0], 0);
		Vec3f dirBC =
			m_projectFittingHeadArray.at<Vec3f>(m_headFaceIndexArray.at<Vec4i>(faceI, 0)[2], 0) -
			m_projectFittingHeadArray.at<Vec3f>(m_headFaceIndexArray.at<Vec4i>(faceI, 0)[1], 0);
		Vec3f dirCD =
			m_projectFittingHeadArray.at<Vec3f>(m_headFaceIndexArray.at<Vec4i>(faceI, 0)[3], 0) -
			m_projectFittingHeadArray.at<Vec3f>(m_headFaceIndexArray.at<Vec4i>(faceI, 0)[2], 0);
		Vec3f dirDA =
			m_projectFittingHeadArray.at<Vec3f>(m_headFaceIndexArray.at<Vec4i>(faceI, 0)[0], 0) -
			m_projectFittingHeadArray.at<Vec3f>(m_headFaceIndexArray.at<Vec4i>(faceI, 0)[3], 0);
		Vec3f normalA = dirDA.cross(dirAB);
		Vec3f normalB = dirAB.cross(dirBC);
		Vec3f normalC = dirBC.cross(dirCD);
		Vec3f normalD = dirCD.cross(dirDA);
		normalA *= 1.f / sqrtf(normalA.dot(normalA));
		normalB *= 1.f / sqrtf(normalB.dot(normalB));
		normalC *= 1.f / sqrtf(normalC.dot(normalC));
		normalD *= 1.f / sqrtf(normalD.dot(normalD));
		Vec3f normal = normalA + normalB + normalC + normalD;
		m_headFaceNormalFlagArray.at<char>(faceI, 0) = normal[2] > 0.f ? 1 : 0;
	}
}

void HeadGenerator::calculateBoundary()
{
	std::vector<int> faceFlag;
	faceFlag.resize(FaceSize);
	// Flag those facing the screen and tagged in boundary model.

	for (int faceI = 0; faceI < FaceSize; faceI++){
		if (m_headFaceBoundaryArray.at<char>(faceI, 0) == -1)
			faceFlag[faceI] = -2;
		else if (m_headFaceBoundaryArray.at<char>(faceI, 0) == 0 || !m_headFaceNormalFlagArray.at<char>(faceI, 0))
			faceFlag[faceI] = -1;
		else
			faceFlag[faceI] = 0;
	}
	// Clean face tags.
	for (int id = 0; id >= -1; id--){
		// Group faces.
		int maxFaceRegion;
		int maxFaceRegionCount = 0;
		int faceRegionNumber = 1;
		for (int faceI = 0; faceI < FaceSize; faceI++){
			if (faceFlag[faceI] != id)
				continue;
			int faceRegionCount = 0;
			std::stack<int> faceStack;
			faceStack.push(faceI);
			while (!faceStack.empty()){
				int currentFace = faceStack.top();
				faceStack.pop();
				if (faceFlag[currentFace] == id){
					faceFlag[currentFace] = faceRegionNumber;
					faceRegionCount++;
					for (int neighborI = 0; neighborI < 4; neighborI++){
						int neighborIndex = m_headFaceNeighborArray.at<Vec4i>(currentFace, 0)[neighborI];
						if(neighborIndex >= 0 && faceFlag[neighborIndex] == id)
							faceStack.push(neighborIndex);
					}
				}
			}
			if (faceRegionCount > maxFaceRegionCount){
				maxFaceRegionCount = faceRegionCount;
				maxFaceRegion = faceRegionNumber;
			}
			faceRegionNumber++;
		}
		faceRegionNumber--;
		// Remove small groups.

		for (int faceI = 0; faceI < FaceSize; faceI++){
			if (faceFlag[faceI] > 0)
				faceFlag[faceI] = (faceFlag[faceI] == maxFaceRegion) ? id : -1 - id;
		}
	}
	// Find boundary vertices and their neighbors.
	std::map<int, int> edges;
	for (int faceI = 0; faceI < FaceSize; faceI++){
		if (faceFlag[faceI] != 0)
			continue;
		Vec4i vertexIndices = m_headFaceIndexArray.at<Vec4i>(faceI, 0);
		std::map<int, int> faceEdges;
		for (int edgeI = 0; edgeI < 4; edgeI++)
			faceEdges[vertexIndices[edgeI]] = vertexIndices[(edgeI + 1) % 4];
		Vec4i neighFaceIndices = m_headFaceNeighborArray.at<Vec4i>(faceI, 0);
		for (int neighborI = 0; neighborI < 4; neighborI++){
			int neighborFaceIndex = neighFaceIndices[neighborI];
			if (neighborFaceIndex < 0 || faceFlag[neighborFaceIndex] == -1)
				continue;
			Vec4i neighborVertexIndices = m_headFaceIndexArray.at<Vec4i>(neighborFaceIndex, 0);
			for (int neighborEdgeI = 0; neighborEdgeI < 4; neighborEdgeI++){
				std::map<int, int>::iterator iter = faceEdges.find(neighborVertexIndices[neighborEdgeI]);
				if (iter != faceEdges.end() && iter->second == neighborVertexIndices[(neighborEdgeI + 3) % 4]){
					faceEdges.erase(iter);
					break;
				}
			}
		}
		for (std::map<int, int>::iterator iter = faceEdges.begin(); iter != faceEdges.end(); iter++){
			if (edges.find(iter->first) != edges.end())
				printf("Boundary edge cross error!\n");
			edges[iter->first] = iter->second;
		}
	}
	// Group boundary vertices.
	m_boundaryVertexNumber = 1;
	int beginVertexI = edges.begin()->first;
	m_boundaryIndexArray.at<int>(0, 0) = beginVertexI;
	int currrentVertexI = beginVertexI;
	while (true){
		int nextVertexI = edges.find(currrentVertexI)->second;
		if (nextVertexI == beginVertexI)
			break;
		m_boundaryIndexArray.at<int>(m_boundaryVertexNumber++, 0) = nextVertexI;
		edges.erase(currrentVertexI);
		currrentVertexI = nextVertexI;
	}
}

void HeadGenerator::pojectionEstimate(bool init)
{
	Vec3f modelMean, imageMean;
	for (int featureI = 0; featureI < m_activeFeatureNumber; featureI++){
		Vec3f modelFeature = m_modelFeatureArray.at<Vec3f>(featureI, 0);
		Vec2f imageFeature = m_imageFeatureArray.at<Vec2f>(featureI, 0);
		modelMean += modelFeature;
		imageMean += Vec3f(imageFeature[0], imageFeature[1], 0.f);
		if (!init)
			imageMean[2] += m_projectModelFeatureArray.at<Vec3f>(featureI, 0)[2];
	}
	modelMean *= 1.f / m_activeFeatureNumber;
	imageMean *= 1.f / m_activeFeatureNumber;
	float modelVariance = 0.f;
	float imageVariance = 0.f;
	for (int featureI = 0; featureI < m_activeFeatureNumber; featureI++){
		Vec3f modelDiff = m_modelFeatureArray.at<Vec3f>(featureI, 0) - modelMean;
		modelVariance += modelDiff.dot(modelDiff);
		Vec2f imageDiff = m_imageFeatureArray.at<Vec2f>(featureI, 0) - Vec2f(imageMean.val);
		imageVariance += imageDiff.dot(imageDiff);
		if (!init){
			float imageDiffZ = m_projectModelFeatureArray.at<Vec3f>(featureI, 0)[2] - imageMean[2];
			imageVariance += imageDiffZ * imageDiffZ;
		}
	}
	modelVariance /= m_activeFeatureNumber;
	imageVariance /= m_activeFeatureNumber;
	Matx33d covarianceMat;
	for (int featureI = 0; featureI < m_activeFeatureNumber; featureI++){
		for (int yI = 0; yI < 3; yI++){
			for (int xI = 0; xI < 2; xI++)
				covarianceMat(xI, yI) +=
					(m_imageFeatureArray.at<Vec2f>(featureI, 0)[xI] - imageMean[xI]) *
					(m_modelFeatureArray.at<Vec3f>(featureI, 0)[yI] - modelMean[yI]);
			if (!init)
				covarianceMat(2, yI) +=
					(m_projectModelFeatureArray.at<Vec3f>(featureI, 0)[2] - imageMean[2]) *
					(m_modelFeatureArray.at<Vec3f>(featureI, 0)[yI] - modelMean[yI]);
		}
	}
	covarianceMat *= 1. / m_activeFeatureNumber;
	SVD svd(covarianceMat);
	Matx33d uMat(svd.u);
	Matx33d vMatT(svd.vt);
	Matx33d sMat;
	for (int yI = 0; yI < 3; yI++)
		for (int xI = 0; xI < 3; xI++)
			sMat(xI, yI) = (xI == yI) ? 1. : 0.;
	if (determinant(covarianceMat) < 0.)
		sMat(2, 2) = -1.;
	Matx33f rotateMat = uMat * sMat * vMatT;
	if (init){
		float sinB = -rotateMat(2, 0);
		float cosB = sqrtf(1.f - sinB * sinB);
		float sinA = rotateMat(2, 1) / cosB;
		float cosA = rotateMat(2, 2) / cosB;
		if (cosA < 0){
			sinA = -sinA;
			cosA = -cosA;
		}
		float cosC = rotateMat(0, 0) / cosB;
		float sinC = rotateMat(1, 0) / cosB;
		if (cosC < 0){
			sinC = -sinC;
			cosC = -cosC;
		}
		rotateMat(0, 0) = cosB * cosC;
		rotateMat(1, 0) = cosB * sinC;
		rotateMat(2, 1) = sinA * cosB;
		rotateMat(2, 2) = cosA * cosB;
		rotateMat(0, 1) = -cosA * sinC + sinA * sinB * cosC;
		rotateMat(0, 2) = sinA * sinC + cosA * sinB * cosC;
		rotateMat(1, 1) = cosA * cosC + sinA * sinB * sinC;
		rotateMat(1, 2) = -sinA * cosC + cosA * sinB * sinC;
	}
	Vec3f translateVec;
	for (int dimI = 0; dimI < 3; dimI++)
		translateVec[dimI] = -Vec3f(rotateMat.row(dimI).val).dot(modelMean);
	float modelImageSum = 0.f;
	float modelSquareSum = 0.f;
	for (int featureI = 0; featureI < m_activeFeatureNumber; featureI++){
		Vec2f modelPoint, imagePoint;
		for (int dimI = 0; dimI < 2; dimI++)
			modelPoint[dimI] = Vec3f(rotateMat.row(dimI).val).dot(m_modelFeatureArray.at<Vec3f>(featureI, 0)) + translateVec[dimI];
		imagePoint = m_imageFeatureArray.at<Vec2f>(featureI, 0) - Vec2f(imageMean.val);
		modelImageSum += modelPoint.dot(imagePoint);
		modelSquareSum += modelPoint.dot(modelPoint);
	}
	float scaleCoefficient = modelImageSum / modelSquareSum;
	translateVec = scaleCoefficient * translateVec + imageMean;
	m_transform.setTransform(scaleCoefficient, translateVec, rotateMat);
}

void HeadGenerator::coefficientEstimate()
{
	Mat pMat = Mat::zeros(TrainSize, TrainSize, CV_32FC1);
	Mat qMat = Mat::zeros(TrainSize, 1, CV_32FC1);
	Transform basisTransform = m_transform;
	basisTransform.setTrans(Vec3f());
	for (int featureI = 0; featureI < m_activeFeatureNumber; featureI++){
		Vec2i featureIndex = m_fittingFeatureIndexArray.at<Vec2i>(featureI, 0);
		float featureWeight = m_featureWeightArray.at<float>(featureI);
		Mat rvkMat = Mat::zeros(2, TrainSize, CV_32FC1);

		for (int trainI = 0; trainI < TrainSize; trainI++){
			Vec3f basisVec = basisTransform * (
				m_basisHeadVectorArray.at<Vec3f>(trainI, featureIndex[0]) +
				m_basisHeadVectorArray.at<Vec3f>(trainI, featureIndex[1])) * 0.5f;
			rvkMat.at<float>(0, trainI) = basisVec[0];
			rvkMat.at<float>(1, trainI) = basisVec[1];
		}
		pMat += featureWeight * rvkMat.t() * rvkMat + m_coefEnergy * m_basisLambdaMatInv;
		Vec2f featureDisp = m_imageFeatureArray.at<Vec2f>(featureI, 0) - Vec2f(m_projectModelFeatureArray.at<Vec3f>(featureI, 0).val);
		for (int trainI = 0; trainI < TrainSize; trainI++)
			qMat.at<float>(trainI, 0) += (float)(featureWeight * rvkMat.col(trainI).dot(Mat(featureDisp)));
	}
	pMat = pMat.inv(cv::DECOMP_CHOLESKY);

	for (int trainI = 0; trainI < TrainSize; trainI++)
		m_basisCoefficientArray.at<float>(trainI, 0) = (float)(pMat.row(trainI).t()).dot(qMat);
}

void HeadGenerator::prepareResultArrays()
{
	if (!m_estimateDone)
		return;

	for (int featureI = 0; featureI < m_activeFeatureNumber; featureI++){
		Vec2f featurePos = Vec2f(m_projectModelFeatureArray.at<Vec3f>(featureI, 0).val);
		featurePos[1] = m_imageHeight - featurePos[1];
		m_resultFeatureArray.at<Vec2f>(featureI, 0) = featurePos;
	}

	for (int boundaryI = 0; boundaryI < m_boundaryVertexNumber; boundaryI++){
		Vec2f vertexPos = Vec2f(m_projectFittingHeadArray.at<Vec3f>(m_boundaryIndexArray.at<int>(boundaryI, 0), 0).val);
		vertexPos[1] = m_imageHeight - vertexPos[1];
		m_boundaryVertexArray.at<Vec2f>(boundaryI, 0) = vertexPos;
	}
	Vec3f bbMin = Vec3f(FLT_MAX, FLT_MAX, FLT_MAX);
	Vec3f bbMax = Vec3f(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (int vertexI = 0; vertexI < VertexSize; vertexI++){
		Vec3f vertex = m_projectFittingHeadArray.at<Vec3f>(vertexI, 0);
		for (int dimI = 0; dimI < 3; dimI++){
			bbMin[dimI] = MIN(bbMin[dimI], vertex[dimI]);
			bbMax[dimI] = MAX(bbMax[dimI], vertex[dimI]);
		}
	}

	m_transform.setCenter((bbMin + bbMax) * 0.5f);

	float maxDisZ = -10000.0f, tmpDisZ;
	float maxDisY = -10000.0f, tmpDisY;
	//float maxDisX = -10000.0f, tmpDisX;

	float maxFittingX = -10000.0f, minFittingX = 10000.0f;
	float maxAverageX = -10000.0f, minAverageX = 10000.0f;

	int vertexIndex;
	int foreheadVertexNum = m_foreheadVertexIndexVec.size();
	for(int i=0; i<foreheadVertexNum; i++)
	{
		vertexIndex = m_foreheadVertexIndexVec[i];
		tmpDisZ = m_fittingHeadArray.at<Vec3f>(vertexIndex, 0)[2] - m_averageHeadArray.at<Vec3f>(vertexIndex, 0)[2];
		if(tmpDisZ > maxDisZ)
			maxDisZ = tmpDisZ;
		tmpDisY = m_fittingHeadArray.at<Vec3f>(vertexIndex, 0)[1] - m_averageHeadArray.at<Vec3f>(vertexIndex, 0)[1];
		if(tmpDisY > maxDisY)
			maxDisY = tmpDisY;
		//tmpDisX = m_fittingHeadArray.at<Vec3f>(vertexIndex, 0)[0] - m_averageHeadArray.at<Vec3f>(vertexIndex, 0)[0];
		//if(tmpDisX > maxDisX)
		   // maxDisX = tmpDisX;

		if(m_fittingHeadArray.at<Vec3f>(vertexIndex, 0)[0] > maxFittingX)
			maxFittingX = m_fittingHeadArray.at<Vec3f>(vertexIndex, 0)[0];
		if(m_fittingHeadArray.at<Vec3f>(vertexIndex, 0)[0] < minFittingX)
			minFittingX = m_fittingHeadArray.at<Vec3f>(vertexIndex, 0)[0];

		if(m_averageHeadArray.at<Vec3f>(vertexIndex, 0)[0] > maxAverageX)
			maxAverageX = m_averageHeadArray.at<Vec3f>(vertexIndex, 0)[0];
		if(m_averageHeadArray.at<Vec3f>(vertexIndex, 0)[0] < minAverageX)
			minAverageX = m_averageHeadArray.at<Vec3f>(vertexIndex, 0)[0];
	}

	LOGI("maxDisZ %f, maxDisY %f\n", maxDisZ, maxDisY);

	m_transform.setDisplacement(cv::Vec3f(maxFittingX-minFittingX, maxDisY, maxDisZ));

}
