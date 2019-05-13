#ifndef HEADGENERATOR_H
#define HEADGENERATOR_H

#include <opencv2/opencv.hpp>

#include "Transform.h"
#include <vector>

class HeadGenerator
{
public:
	HeadGenerator					(const char * featureFile, const char * trainResultFile, const char * auxiliaryDataFile, const char * foreheadVertexIndexFile);
	~HeadGenerator					();

	void		reset				();

	void		save				(const char * faceTexAndTopoFile, const char* boundPath, const char *coefPath, const char *modelPath);

	void		setFeatures			(cv::Mat *features, int width, int height);

	void		runHeadGenerate		();

private:
	void		resetModel			();
	void		updateModel			();
	void		updateBoundary		();
	void		updateFeature		();
	void		updateNormal		();
	void		calculateBoundary	();
	void		pojectionEstimate	(bool init);
	void		coefficientEstimate	();
	void		prepareResultArrays	();

	int			m_imageWidth;
	int			m_imageHeight;

	int			m_estimateDone;
	int			m_activeFeatureNumber;
	int			m_boundaryFaceNumber;
	int			m_interiorFaceNumber;
	int			m_boundaryVertexNumber;

	cv::Mat		m_defaultFeatureIndexArray;
	cv::Mat		m_fittingFeatureIndexArray;
	cv::Mat		m_featureWeightArray;
	cv::Mat		m_featureActiveArray;
	cv::Mat		m_averageHeadArray;
	cv::Mat		m_headFaceNeighborArray;
	cv::Mat		m_headFaceIndexArray;
	cv::Mat		m_headFaceBoundaryArray;
	cv::Mat		m_headFaceNormalFlagArray;
	cv::Mat		m_boundaryIndexArray;
	cv::Mat		m_basisHeadValueArray;
	cv::Mat		m_basisHeadVectorArray;
	cv::Mat		m_basisCoefficientArray;
	cv::Mat		m_basisLambdaMatInv;
	cv::Mat		m_fittingHeadArray;
	cv::Mat		m_projectFittingHeadArray;
	cv::Mat		m_modelFeatureArray;
	cv::Mat		m_projectModelFeatureArray;
	cv::Mat		m_imageFeatureArray;
	cv::Mat		m_resultFeatureArray;
	cv::Mat		m_boundaryVertexArray;

	Transform	m_transform;

	int			m_estimateIteration;
	int			m_projectionIteration;
	float		m_coefEnergy;
    
    std::vector<int>	m_foreheadVertexIndexVec;
};

#endif	// HEADGENERATOR_H
